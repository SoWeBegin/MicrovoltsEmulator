#ifndef CAST_HOST_ROOM_HOST_JOIN_CONTROL_HANDLER
#define CAST_HOST_ROOM_HOST_JOIN_CONTROL_HANDLER

#include "Network/Session.h"
#include "../Network/CastSession.h"
#include "../Classes/RoomsManager.h"

namespace Cast
{
    namespace Handlers
    {
        // Client sends packet to the host whenever someone joins their room

        // Ok. checked
        inline void handlePlayerJoinRoom(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            const std::uint64_t hostSessionId = request.getSession();
            const std::uint64_t selfSessionId = session.getId();

            std::cout << "Player: " << selfSessionId << " attempting to join room of player: " << hostSessionId << '\n';
            roomsManager.switchRoomJoinOrExit(session, hostSessionId);

           // roomsManager.printRoomInfo(session.getRoomId(), "after PlayerJoinRoom");
        }
    }
}

#endif