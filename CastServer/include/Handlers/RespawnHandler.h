#ifndef PLAYER_RESPAWN_HANDLER_H
#define PLAYER_RESPAWN_HANDLER_H

#include <Network/Session.h>
#include "../Network/CastSession.h"
#include "../../../MainServer/include/Structures/AccountInfo/MainAccountUniqueId.h"

#include <boost/interprocess/mapped_region.hpp> 
#include <chrono> 
#include "../../../MainServer/include/Structures/AccountInfo/MainAccountInfo.h"

#include <thread>
#include "../Network/SessionsManager.h"

#include "Utils/Logger.h"

namespace Cast
{
    namespace Handlers
    {

        // After the non host request, the host sends the respawn packet to the target player
        inline void handlePlayerRespawn(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            Common::Network::Packet response;
            response.setTcpHeader(session.getId(), Common::Enums::NO_ENCRYPTION);
            response.setOrder(request.getOrder());

            struct PlayerRespawnPosition
            {
                std::uint16_t x = 0;
                std::uint16_t y = 0;
                std::uint16_t z = 0; 
                std::uint16_t w = 0;
                Main::Structures::UniqueId targetUniqueId{};
            } playerRespawnPosition;

            std::memcpy(&playerRespawnPosition, request.getData(), sizeof(playerRespawnPosition));
            playerRespawnPosition.z += 70; // add some more height 
            response.setData(reinterpret_cast<std::uint8_t*>(&playerRespawnPosition), sizeof(playerRespawnPosition));    
            
            const auto targetSessionId = playerRespawnPosition.targetUniqueId.session;
            roomsManager.broadcastToRoom(session.getId(), response);

           // roomsManager.printRoomInfo(session.getRoomId(), "After handle Player Respawn");
        }

        // Non host client requests host to respawn them
        inline void handlePlayerRespawnRequest(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            auto& logger = ::Utils::Logger::getInstance(true);
            logger.log("Respawning player", ::Utils::LogType::Info, "handlePlayerRespawnRequest");

            const auto roomHostSessionId = request.getSession();
            const auto selfSessionId = session.getId();

            roomsManager.playerForwardToHost(roomHostSessionId, selfSessionId, const_cast<Common::Network::Packet&>(request));

            //roomsManager.printRoomInfo(session.getRoomId(), "After handle Player Respawn Req");

        }
    }
}

#endif
