#ifndef CAST_CRASH_HANDLER_H
#define CAST_CRASH_HANDLER_H

#include "Network/Session.h"
#include "../Network/CastSession.h"
#include "../Classes/RoomsManager.h"

#include <Utils/Logger.h>

namespace Cast
{
    namespace Handlers
    {
        inline void handleCrash(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            const auto selfSessionId = session.getId();
            const auto roomHostId = request.getSession();
            Main::Structures::UniqueId uniqueId{ selfSessionId, 4, 0 };
            auto response = request;
            response.setData(reinterpret_cast<std::uint8_t*>(&uniqueId), sizeof(uniqueId));
            roomsManager.playerForwardToHost(roomHostId, selfSessionId, response);
            
            // This sets is in match to false, roomNum to -1, removes player from room, removes player from roomsManager's unordered_map
            roomsManager.removePlayerFromRoom(session.getId());
        }
    }
}

#endif
