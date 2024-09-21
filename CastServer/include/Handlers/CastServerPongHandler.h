#ifndef CAST_SERVER_PONG_H
#define CAST_SERVER_PONG_H

#include <Network/Session.h>
#include <Utils/Logger.h>

namespace Cast
{
    namespace Handlers
    {
        inline void pongHandler(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            Common::Network::Packet response;
            response.setTcpHeader(request.getSession(), Common::Enums::NO_ENCRYPTION);
            response.setOrder(72);
            response.setMission(request.getMission());         
            response.setOption(request.getOption());
            session.asyncWrite(response);
        }
    }
}

#endif