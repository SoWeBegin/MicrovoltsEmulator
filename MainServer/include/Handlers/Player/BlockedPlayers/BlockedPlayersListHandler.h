#ifndef MAIN_BLOCKEDPLAYER_LIST_HANDLER_H
#define MAIN_BLOCKEDPLAYER_LIST_HANDLER_H

#include "../../../../include/Network/MainSessionManager.h"
#include "../../../../include/Structures/PlayerLists/BlockedPlayer.h"
#include <vector>

namespace Main
{
    namespace Handlers
    {
        inline void handleBlockedPlayerList(const Common::Network::Packet& request, Main::Network::Session& session)
        {
            std::vector<Main::Structures::BlockedPlayer> blockedPlayers = session.getBlockedPlayers();

            Common::Network::Packet response;
            response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
            response.setCommand(request.getOrder(), 0, 37, blockedPlayers.size());  // max blocked players is 30, should never exceed 1440 bytes in total
            response.setData(reinterpret_cast<std::uint8_t*>(blockedPlayers.data()), blockedPlayers.size() * sizeof(Main::Structures::BlockedPlayer));
            session.asyncWrite(response);
        }
    }
}

#endif