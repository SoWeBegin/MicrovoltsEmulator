#ifndef ACCOUNT_NAME_HANDLER_HEADER
#define ACCOUNT_NAME_HANDLER_HEADER

#include "../Network/CastSession.h"
#include "../Classes/RoomsManager.h"

namespace Cast
{
    namespace Handlers
    {
        inline void handleAccountNames(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            // This should only be broadcast to players that are NOT inside the match, but inside the room!
            //roomsManager.broadcastToRoomExceptSelf(session.getId(), session.getRoomId(), const_cast<Common::Network::Packet&>(request)); // note: do not broadcast to self!

           // roomsManager.printRoomInfo(session.getRoomId(), "after HandleAccountNames");
        }
    }
}

#endif