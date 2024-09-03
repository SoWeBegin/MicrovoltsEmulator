#ifndef ROOM_TICK_PROVIDER_HANDLER_H
#define ROOM_TICK_PROVIDER_HANDLER_H

#include <Network/Session.h>
#include "../Network/CastSession.h"
#include "../../../MainServer/include/Structures/AccountInfo/MainAccountUniqueId.h"
#include "../Classes/RoomsManager.h"

#include "Utils/Logger.h"

namespace Cast
{
    namespace Handlers
    {
        // After the host client receives packet 78 from the non-host, it provides the non-host with the updated room tick
        // Without this handler, the player never respawn (not even TAB shows anything) => The player keeps being in a waiting initial state.
        inline void handleRoomTickProviding(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            const auto receiverSessionId = request.getSession();
            const auto hostSessionId = session.getId();

            std::cout << "Host: " << hostSessionId << " sending tick to: " << receiverSessionId << '\n';
            roomsManager.hostForwardToPlayer(hostSessionId, receiverSessionId, const_cast<Common::Network::Packet&>(request));

           // roomsManager.printRoomInfo(session.getRoomId(), "After handle room tick providing");

        }
    }
}

#endif