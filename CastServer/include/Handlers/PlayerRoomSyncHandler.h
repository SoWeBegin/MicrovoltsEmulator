#ifndef PLAYER_ROOM_SYNC_HANDLER_H
#define PLAYER_ROOM_SYNC_HANDLER_H

#include <Network/Session.h>
#include "../Network/CastSession.h"
#include "../../../MainServer/include/Structures/AccountInfo/MainAccountUniqueId.h"
#include "../Classes/RoomsManager.h"


namespace Cast
{
    namespace Handlers
    {
        // This is needed, otherwise the player:
        // 1. does not get the time left of the match, and
        // 2. they don't respawn at all.
        inline void handlePlayerSyncWithRoom(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            auto& logger = ::Utils::Logger::getInstance();
            logger.log("Getting player sync room info", ::Utils::LogType::Info, "handlePlayerSyncWithRoom");

            const auto receiverSessionId = request.getSession();
            const auto hostSessionId = session.getId();
            roomsManager.hostForwardToPlayer(hostSessionId, receiverSessionId, const_cast<Common::Network::Packet&>(request));

           // roomsManager.printRoomInfo(session.getRoomId(), "After handlePlayerSyncWithRoom");
        }
    }
}

#endif