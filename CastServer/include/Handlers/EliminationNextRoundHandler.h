#ifndef ELIMINATION_NEXTROUND_HANDLER_H
#define ELIMINATION_NEXTROUND_HANDLER_H

#include <Network/Session.h>
#include "../Network/CastSession.h"
#include "../../../MainServer/include/Structures/AccountInfo/MainAccountUniqueId.h"
#include "../Classes/RoomsManager.h"


namespace Cast
{
    namespace Handlers
    {
        inline void handleEliminationNextRound(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            // Just ignore this on the cast server, otherwise it all stops working for some reason...
            //roomsManager.printRoomInfo(session.getRoomId(), "After EliminationNextRoundHandler");
        }
    }
}

#endif