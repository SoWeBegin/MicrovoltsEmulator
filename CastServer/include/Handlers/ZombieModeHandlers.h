#ifndef ZOMBIE_MODE_HANDLERS_H
#define ZOMBIE_MODE_HANDLERS_H

#include <Network/Session.h>
#include "../Network/CastSession.h"
#include "../../../MainServer/include/Structures/AccountInfo/MainAccountUniqueId.h"
#include "../Classes/RoomsManager.h"

namespace Cast
{
	namespace Handlers
	{
        inline void handleItemRespawn(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            roomsManager.broadcastToRoom(session.getId(), const_cast<Common::Network::Packet&>(request));
        }

        inline void handleZombieRespawn(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            roomsManager.broadcastToRoom(session.getId(), const_cast<Common::Network::Packet&>(request));
        }

        template<Common::Enums::PlayerType PlayerType>
        inline void handleItemPickup(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            if constexpr (PlayerType == Common::Enums::HOST)
            {
                roomsManager.broadcastToRoom(session.getId(), const_cast<Common::Network::Packet&>(request));
            }
            else if constexpr (PlayerType == Common::Enums::NON_HOST)
            {
                const auto hostSessionId = request.getSession();
                const auto senderId = session.getId();
                roomsManager.playerForwardToHost(hostSessionId, senderId, const_cast<Common::Network::Packet&>(request));
            }
        }

        template<Common::Enums::PlayerType PlayerType>
        inline void handleZombieAbility(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            if constexpr (PlayerType == Common::Enums::HOST)
            {
                roomsManager.broadcastToRoom(session.getId(), const_cast<Common::Network::Packet&>(request));
            }
            else if constexpr (PlayerType == Common::Enums::NON_HOST)
            {
                const auto hostSessionId = request.getSession();
                const auto senderId = session.getId();
                roomsManager.playerForwardToHost(hostSessionId, senderId, const_cast<Common::Network::Packet&>(request));
            }
        }
	}
}

#endif
