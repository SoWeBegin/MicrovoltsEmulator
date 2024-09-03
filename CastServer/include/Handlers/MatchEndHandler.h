#ifndef MATCH_END_HANDLER_H
#define MATCH_END_HANDLER_H

#include "../Network/CastSession.h"
#include "../Classes/RoomsManager.h"

namespace Cast
{
    namespace Handlers
    {
        inline void handleMatchEnd(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            roomsManager.endMatch(session.getId());

           // roomsManager.printRoomInfo(session.getRoomId(), "After HandleMatchEnd");
        }
    }
}

#endif