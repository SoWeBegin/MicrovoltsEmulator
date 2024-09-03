#ifndef LOBBY_CLAN_LIST_HANDLER_H
#define LOBBY_CLAN_LIST_HANDLER_H

#include <Network/Session.h>
#include <vector>
#include <unordered_map>
#include "../Structures/PlayerLists/SingleLobbyClanList.h"

namespace Main
{
    namespace Handlers
    {
        inline void handleLobbyClanList(const Common::Network::Packet& request, Common::Network::Session& session, Main::Network::SessionsManager& sessionsManager)
        {
            const auto& allSessions = sessionsManager.getAllSessions();

            Common::Network::Packet response;
            response.setTcpHeader(request.getSession(), Common::Enums::NO_ENCRYPTION);
            response.setOrder(request.getOrder());

            if (allSessions.empty() || allSessions.size() == 1)
            {
                response.setData(nullptr, 0);
                response.setOption(0);
                response.setExtra(6);
                session.asyncWrite(response);
                return;
            }
            auto selfAccountInfo = allSessions.at(session.getId())->getAccountInfo();
            std::vector<Main::Structures::SingleLobbyClanList> playerList;

            for (const auto& [sessionId, currentSession] : allSessions)
            {
                if (sessionId == session.getId()) continue; // Skip self user
            
                const auto& partialAccountData = currentSession->getAccountInfo();

                if (partialAccountData.clanId != selfAccountInfo.clanId) continue; // Skip non clan members

                Main::Structures::SingleLobbyClanList singlePlayerList;
            
                singlePlayerList.level = partialAccountData.playerLevel;
                strncpy(singlePlayerList.name, partialAccountData.nickname, sizeof(singlePlayerList.name));
                singlePlayerList.uniqueId.server = partialAccountData.uniqueId.server;
                singlePlayerList.uniqueId.session = partialAccountData.uniqueId.session;
                singlePlayerList.uniqueId.unknown = partialAccountData.uniqueId.unknown;
            
                playerList.push_back(singlePlayerList);
            }

            response.setOption(playerList.size());
            response.setExtra(37);
            response.setData(reinterpret_cast<std::uint8_t*>(playerList.data()), playerList.size() * sizeof(Main::Structures::SingleLobbyClanList));
            session.asyncWrite(response);
        }
    }
}

#endif