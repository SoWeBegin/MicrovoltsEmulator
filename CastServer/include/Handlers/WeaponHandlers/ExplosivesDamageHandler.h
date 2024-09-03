#ifndef EXPLOSIVES_DAMAGE_HANDLER_H
#define EXPLOSIVES_DAMAGE_HANDLER_H

#include "../../Network/CastSession.h"
#include "../../Network/SessionsManager.h"
#include "../../Classes/RoomsManager.h"

namespace Cast
{
    namespace Handlers
    {
        inline void handleExplosivesDamage(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            roomsManager.broadcastToRoom(session.getId(), const_cast<Common::Network::Packet&>(request));
        }
    }
}

#endif