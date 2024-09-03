#ifndef MAIN_LOBBY_LIST_HANDLER_H
#define MAIN_LOBBY_LIST_HANDLER_H

#include <Network/Session.h>
#include <vector>
#include <unordered_map>
#include "../Structures/PlayerLists/SingleLobbyList.h"

namespace Main
{
    namespace Handlers
    {
        inline void handleLobbyUserList(const Common::Network::Packet& request, Common::Network::Session& session, Main::Network::SessionsManager& sessionsManager)
        {
            const auto& allSessions = sessionsManager.getAllSessions();

            Common::Network::Packet response;
            response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
            response.setOrder(request.getOrder());
            response.setOption(allSessions.size());

            if (allSessions.empty() || allSessions.size() == 1)
            {
                response.setData(nullptr, 0);
                response.setExtra(6);
                session.asyncWrite(response);
                return;
            }

            std::vector<Main::Structures::SinglePlayerInfoList> playerList;
            for (const auto& [sessionId, currentSession] : allSessions)
            {
                if (sessionId == session.getId() || !currentSession->isInLobby()) continue; // We don't want to show the user itself in the lobby's player list...
                const auto& partialAccountData = currentSession->getAccountInfo();
                Main::Structures::SinglePlayerInfoList singlePlayerList;

                singlePlayerList.clanLogoBackId = partialAccountData.clanLogoBackId;
                singlePlayerList.clanLogoFrontId = partialAccountData.clanLogoFrontId;
                singlePlayerList.level = partialAccountData.playerLevel;
                strncpy(singlePlayerList.name, partialAccountData.nickname, sizeof(singlePlayerList.name));
                singlePlayerList.uniqueId.server = partialAccountData.uniqueId.server;
                singlePlayerList.uniqueId.session = partialAccountData.uniqueId.session;
                singlePlayerList.uniqueId.unknown = partialAccountData.uniqueId.unknown;

                playerList.push_back(singlePlayerList);
            }

            const std::size_t totalBytes = playerList.size() * sizeof(Main::Structures::SinglePlayerInfoList);
            constexpr std::size_t MAX_PACKET_SIZE = 1440;
            constexpr std::size_t headerSize = sizeof(Common::Protocol::TcpHeader) + sizeof(Common::Protocol::CommandHeader);

            if (totalBytes < MAX_PACKET_SIZE)
            {
                response.setExtra(37);
                response.setData(reinterpret_cast<std::uint8_t*>(playerList.data()), totalBytes);
                session.asyncWrite(response);
                return;
            }
            else
            {
                std::size_t currentPlayerIndex = 0;
                std::uint16_t packetExtra = 0;
                const std::size_t maxPayloadSize = MAX_PACKET_SIZE - headerSize;
                const std::size_t playersToSend = maxPayloadSize / sizeof(Main::Structures::SinglePlayerInfoList);

                while (currentPlayerIndex < allSessions.size())
                {
                    // WARNING: SET OPTION! (setOption(allSessions.size() - currentPlayerIndex) => test it
                    std::vector<Main::Structures::SinglePlayerInfoList> currentPlayerListPacket(
                        playerList.begin() + currentPlayerIndex,
                        playerList.begin() + std::min(currentPlayerIndex + playersToSend, playerList.size())
                    );

                    packetExtra = currentPlayerIndex == 0 ? 37 : 0;
                    response.setExtra(packetExtra);
                    response.setData(reinterpret_cast<std::uint8_t*>(currentPlayerListPacket.data()), currentPlayerListPacket.size() * sizeof(Main::Structures::SinglePlayerInfoList));
                    response.setOption(currentPlayerListPacket.size());
                    session.asyncWrite(response);

                    currentPlayerIndex += playersToSend;
                }
            }
        }
    }
}

#endif