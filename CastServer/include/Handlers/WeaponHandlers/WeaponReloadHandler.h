#ifndef WEAPON_RELOAD_HANDLER_H
#define WEAPON_RELOAD_HANDLER_H

#include "../../Network/CastSession.h"
#include "../../Network/SessionsManager.h"

namespace Cast
{
    namespace Handlers
    {
        inline void handleWeaponReload(const Common::Network::Packet& request, Cast::Network::Session& session)
        {
            session.asyncWrite(const_cast<Common::Network::Packet&>(request));
        }
    }
}

#endif