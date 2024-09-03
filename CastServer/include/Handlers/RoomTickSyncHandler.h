#ifndef ROOM_TICK_SYNC_HANDLER_H
#define ROOM_TICK_SYNC_HANDLER_H

#include <Network/Session.h>
#include "../Network/CastSession.h"
#include "../../../MainServer/include/Structures/AccountInfo/MainAccountUniqueId.h"
#include "../Classes/RoomsManager.h"
#include <Utils/Logger.h>


namespace Cast
{
    namespace Handlers
    {
        /*
           When Option==9 in packet order 257: the non host's client sends packet 79 to the server, which dispatches to the host
           This packet asks the host to provide the room sync to the non-host
        */
        inline void handleRoomTickSyncRequest(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            std::cout << "Player: " << session.getId() << " asking Tick Request to: " << request.getSession() << '\n';

            const auto roomHostSessionId = request.getSession();
            const auto selfSessionId = session.getId();

            roomsManager.playerForwardToHost(roomHostSessionId, selfSessionId, const_cast<Common::Network::Packet&>(request));

            //roomsManager.printRoomInfo(session.getRoomId(), "After handle room tick sync req");
        }
    }
}

#endif