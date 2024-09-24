#ifndef LOBBY_CLAN_LIST_HANDLER_H
#define LOBBY_CLAN_LIST_HANDLER_H

#include <Network/Session.h>
#include "../Structures/PlayerLists/SingleLobbyClanList.h"
#include <vector>

namespace Main
{
    namespace Handlers
    {
        inline void handleLobbyClanList(const Common::Network::Packet& request, Common::Network::Session& session, Main::Network::SessionsManager& sessionsManager)
        {
            Common::Network::Packet response;
            response.setTcpHeader(request.getSession(), Common::Enums::NO_ENCRYPTION);

            const auto& allSessions = sessionsManager.getAllSessions();
            if (allSessions.empty() || allSessions.size() == 1)
            {
                response.setCommand(request.getOrder(), 0, 6, 0);
                session.asyncWrite(response);
            }
            else
            {
                auto selfAccountInfo = allSessions.at(session.getId())->getAccountInfo();
                std::vector<Main::Structures::SingleLobbyClanList> playerList;

                for (const auto& [sessionId, currentSession] : allSessions)
                {
                    if (sessionId == session.getId()) continue; // Skip self user

                    const auto& partialAccountData = currentSession->getAccountInfo();
                    if (partialAccountData.clanId != selfAccountInfo.clanId) continue; // Skip non clan members

                    Main::Structures::SingleLobbyClanList singlePlayerList;
                    singlePlayerList.level = partialAccountData.playerLevel;
                    strcpy_s(singlePlayerList.name, partialAccountData.nickname);
                    singlePlayerList.uniqueId.server = partialAccountData.uniqueId.server;
                    singlePlayerList.uniqueId.session = partialAccountData.uniqueId.session;
                    singlePlayerList.uniqueId.handlePlayerInvite = partialAccountData.uniqueId.handlePlayerInvite;
                    playerList.push_back(singlePlayerList);
                }

                response.setCommand(request.getOrder(), 0, 37, playerList.size());
                response.setData(reinterpret_cast<std::uint8_t*>(playerList.data()), playerList.size() * sizeof(Main::Structures::SingleLobbyClanList));
                session.asyncWrite(response);
            }
        }
    }
}

#endif
