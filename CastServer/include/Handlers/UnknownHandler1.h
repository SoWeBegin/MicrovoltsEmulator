#ifndef UNKNOWN_HANDLER1_H
#define UNKNOWN_HANDLER1_H

#include <Network/Session.h>
#include "../Network/CastSession.h"
#include "../../../MainServer/include/Structures/AccountInfo/MainAccountUniqueId.h"
#include "../Classes/RoomsManager.h"

#include "Utils/Logger.h"
namespace Cast
{
    namespace Handlers
    {
        // This is a request from the client to get the Room Info (e.g. how many wins in which team, etc)
        inline void roomInfoHandler(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            const auto roomHostSessionId = request.getSession();
            const auto selfSessionId = session.getId();
            // roomsManager.playerForwardToHost(session.getRoomId(), selfSessionId, const_cast<Common::Network::Packet&>(request));
            // This command does NOT exist in the client! [order = 255]
        }
    }
}

#endif