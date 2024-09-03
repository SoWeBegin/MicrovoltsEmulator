#ifndef WEAPON_HOST_ATTACK_HANDLER_H
#define WEAPON_HOST_ATTACK_HANDLER_H

#include "../../Network/CastSession.h"
#include "../../Classes/RoomsManager.h"
#include <DirectXPackedVector.h>

namespace Cast
{
    namespace Handlers
    {
        inline void handleHostWeaponAttack(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            roomsManager.broadcastToRoom(session.getId(), const_cast<Common::Network::Packet&>(request));
        }

        inline void nonHostWeaponAttack(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            roomsManager.playerForwardToHost(request.getSession(), session.getId(), const_cast<Common::Network::Packet&>(request));
        }
    }
}

#endif