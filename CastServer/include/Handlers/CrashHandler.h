#ifndef CAST_CRASH_HANDLER_H
#define CAST_CRASH_HANDLER_H

#include "Network/Session.h"
#include "../Network/CastSession.h"
#include "../Classes/RoomsManager.h"

#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_upgradable_mutex.hpp>

#include <iostream>
#include <Utils/Logger.h>
#include "Utils/IPC_Structs.h"

namespace Cast
{
    namespace Handlers
    {
        inline void handleCrash(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            auto& logger = ::Utils::Logger::getInstance(true);
            logger.log<::Utils::LogDestination::File>("User crashed (SessionID: " + std::to_string(session.getId()) + ", RoomNum: "
                + std::to_string(session.getRoomNumber()), ::Utils::LogType::Info, "handleCrash");

            const auto selfSessionId = session.getId();
            const auto roomHostId = request.getSession();
            Main::Structures::UniqueId uniqueId{ selfSessionId, 4, 0 };
            auto response = request;
            response.setData(reinterpret_cast<std::uint8_t*>(&uniqueId), sizeof(uniqueId));
            roomsManager.playerForwardToHost(roomHostId, selfSessionId, response);
            
            // This sets is in match to false, roomNum to -1, removes player from room, removes player from roomsManager's unordered_map
            roomsManager.removePlayerFromRoom(session.getId());
              
            //roomsManager.printRoomInfo(session.getRoomId(), "After CrashHandler");
        }
    }
}

#endif
