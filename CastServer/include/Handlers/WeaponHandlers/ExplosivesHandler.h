#ifndef EXPLOSIVES_HANDLER_H
#define EXPLOSIVES_HANDLER_H

#include "../../Network/CastSession.h"
#include "../../Classes/RoomsManager.h"
#include <DirectXPackedVector.h>

namespace Cast
{
    namespace Handlers
    {
        inline void handleExplosives(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            roomsManager.broadcastToRoom(session.getId(), const_cast<Common::Network::Packet&>(request));
        }
    }
}

#endif