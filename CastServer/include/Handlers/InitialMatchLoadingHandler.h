#ifndef INITIAL_MATCH_LOADING_HANDLER_H
#define INITIAL_MATCH_LOADING_HANDLER_H

#include <Network/Session.h>
#include "../Network/CastSession.h"
#include "../../../MainServer/include/Structures/AccountInfo/MainAccountUniqueId.h"
#include "../Classes/RoomsManager.h"
#include "../Classes/RoomsManager.h"

#include "Utils/Logger.h"
#include "Utils/IPC_Structs.h"

#include "RoomTickSyncHandler.h"

namespace Cast
{
    namespace Handlers
    {
        inline void handleMatchInitialLoading(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            Common::Network::Packet response;
            response.setTcpHeader(request.getSession(), Common::Enums::NO_ENCRYPTION);
            response.setOrder(request.getOrder());
            Main::Structures::UniqueId uniqueId{ session.getId(), 4, 0};
            response.setOption(request.getOption());
            response.setData(reinterpret_cast<std::uint8_t*>(&uniqueId), sizeof(uniqueId));
            roomsManager.broadcastToRoomExceptSelf(session.getId(), response);
      
            session.setIsInMatch(true);
        }
    }
}

#endif