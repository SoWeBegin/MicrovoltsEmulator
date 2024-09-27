#ifndef CAST_ROOM_CREATION_HANDLER_H
#define CAST_ROOM_CREATION_HANDLER_H

#include "Network/Session.h"
#include "../Network/CastSession.h"
#include "../Network/SessionsManager.h"
#include "../Classes/Room.h"
#include "../Classes/RoomsManager.h"

namespace Cast
{
    namespace Handlers
    {
        // Ok. Checked.
        inline void handleRoomCreation(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            // Create a room, add the player to id (sessionID=>Session pair).
            // Then, add the room to roomsManager. Pass the room and the sessionID of the player. Don't keep track that it's the host: the client does this for us in P2P.
            roomsManager.addRoom(std::make_shared<Cast::Classes::Room>(session.getId(), &session), session.getId());
        }
    }
}

#endif