#ifndef MATCH_LEAVE_HANDLER_H
#define MATCH_LEAVE_HANDLER_H

#include "../Network/CastSession.h"
#include "../Classes/RoomsManager.h"


namespace Cast
{
    namespace Handlers
    {
        inline void handleMatchLeave(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            const auto selfSessionId = session.getId();

            auto& logger = ::Utils::Logger::getInstance();
            logger.log("Player is leaving the match (SessionID: " + std::to_string(session.getId())
                + ", roomNum: " + std::to_string(session.getRoomNumber()), ::Utils::LogType::Info, "handleMatchLeave");

            Main::Structures::UniqueId uniqueId{ selfSessionId, 4, 0 };
            auto response = request;
            response.setData(reinterpret_cast<std::uint8_t*>(&uniqueId), sizeof(uniqueId));

            session.setIsInMatch(false);
      

            //roomsManager.printRoomInfo(session.getRoomId(), "after PlayerLeaveMatch");
        }
    }
}

#endif
