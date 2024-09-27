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
            Main::Structures::UniqueId uniqueId{ selfSessionId, 4, 0 };
            auto response = request;
            response.setData(reinterpret_cast<std::uint8_t*>(&uniqueId), sizeof(uniqueId));

            session.setIsInMatch(false);
        }
    }
}

#endif
