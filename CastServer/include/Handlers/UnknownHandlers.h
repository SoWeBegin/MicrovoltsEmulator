#ifndef UNKNOWN_HANDLER_4_H
#define UNKNOWN_HANDLER_4_H

#include <Network/Session.h>
#include "../Network/CastSession.h"
#include "../../../MainServer/include/Structures/AccountInfo/MainAccountUniqueId.h"
#include "../Classes/RoomsManager.h"
#include "../../../MainServer/include/CdbUtils.h"
#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/interprocess/creation_tags.hpp>

#include "Utils/Logger.h"

namespace Cast
{
    namespace Handlers
    {
        inline void handleMatchStart(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            auto& logger = ::Utils::Logger::getInstance();
            logger.log("Handling match start", ::Utils::LogType::Info, "handleMatchStart");

            session.setIsInMatch(true);

            /*auto mapIdInfoFromMain = ::Utils::IPCManager::ipc_castGetFromMain<::Utils::MapInfo>(std::to_string(session.getRoomNumber()), "map_info");
            auto opt = roomsManager.getHostIdByRoomNumber(session.getRoomNumber());
            if (opt != std::nullopt)
            {
                roomsManager.setMapFor(*opt, mapIdInfoFromMain.mapId);
            }
            else
            {
                std::cout << "HandleMatchStart: Nullopt for mapIdInfoFromMain!\n";
            }

            auto modeIfInfoFromMain = ::Utils::IPCManager::ipc_castGetFromMain<::Utils::ModeInfo>(std::to_string(session.getRoomNumber()), "mode_info");
            opt = roomsManager.getHostIdByRoomNumber(session.getRoomNumber());
            if (opt != std::nullopt)
            {
                roomsManager.setModeFor(*opt, modeIfInfoFromMain.modeId);
            }
            else
            {
                std::cout << "HandleMatchStart: Nullopt for modeIfInfoFromMain!\n";
            }*/

            const auto receiverSessionId = request.getSession();
            const auto hostSessionId = session.getId();

            // For some reason, whenever a new round starts in elimination, the client resends this packet to the host
            // And if we resend this packet to the host client, it resets the score to 0... so we just avoid doing that.
            if (hostSessionId != receiverSessionId)
            {
                logger.log("session.getRoomId() != receiverSessionId", ::Utils::LogType::Info, "handleMatchStart");
                roomsManager.hostForwardToPlayer(hostSessionId, receiverSessionId, const_cast<Common::Network::Packet&>(request));
            }

            //roomsManager.printRoomInfo(session.getRoomId(), "After Handle Match Start");
        }

        // order 94
        inline void unknownHandler3(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            auto& logger = ::Utils::Logger::getInstance();
            logger.log("Unknown Handler 3", ::Utils::LogType::Info, "unknownHandler3");

            const auto receiverSessionId = request.getSession();
            const auto hostSessionId = session.getId();

            roomsManager.broadcastToRoomExceptSelf(hostSessionId, const_cast<Common::Network::Packet&>(request));

            //roomsManager.printRoomInfo(session.getRoomId(), "After unknownHandler3");
        }

        inline void unknownHandler306(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {

            const auto receiverSessionId = request.getSession();
            const auto hostSessionId = session.getId();
            roomsManager.hostForwardToPlayer(hostSessionId, receiverSessionId, const_cast<Common::Network::Packet&>(request));

            std::cout << "(unknownHandler306) HostSessionID: " << hostSessionId << " => playerSessionID: " << receiverSessionId << '\n';

          //  roomsManager.printRoomInfo(session.getRoomId(), "After unknownHandler306");
        }

        // CTB battery respawn when host changes, Bomb battle bomb plant,
        inline void similarHandlers(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            const auto hostSessionId = request.getSession();
            const auto selfId = session.getId();
            roomsManager.broadcastToRoom(session.getId(), const_cast<Common::Network::Packet&>(request));

            std::cout << "(unknownHandler90) HostSessionID: " << hostSessionId << " => playerSessionID: " << selfId << '\n';

            //  roomsManager.printRoomInfo(session.getRoomId(), "After unknownHandler306");
        }

        inline void playerToHost(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            const auto hostSessionId = request.getSession();
            const auto selfId = session.getId();
            roomsManager.playerForwardToHost(hostSessionId, selfId, const_cast<Common::Network::Packet&>(request));
            //roomsManager.broadcastToRoom(session.getId(), const_cast<Common::Network::Packet&>(request));

            std::cout << "(unknownHandler90) HostSessionID: " << hostSessionId << " => playerSessionID: " << selfId << '\n';

            //  roomsManager.printRoomInfo(session.getRoomId(), "After unknownHandler306");
        }
    }
}

#endif