#ifndef CAST_SWITCH_STATE_HANDLER_H
#define CAST_SWITCH_STATE_HANDLER_H

#include "Network/Session.h"
#include "../Network/CastSession.h"
#include "../Classes/RoomsManager.h"

namespace Cast
{
    namespace Handlers
    {
        // Ok. checked.
        inline void leaveRoom(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            roomsManager.switchRoomJoinOrExit(session);
          //  roomsManager.printRoomInfo(session.getRoomId(), "After switch state handler");
        }
    }
}

#endif