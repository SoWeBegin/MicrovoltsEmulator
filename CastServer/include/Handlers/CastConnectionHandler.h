#ifndef CAST_CONNECTION_HANDLER_H
#define CAST_CONNECTION_HANDLER_H

#include "Network/Session.h"
#include "../Network/CastSession.h"

#include <iostream>
namespace Cast
{
    namespace Handlers
    {
        inline void connectionHandler(const Common::Network::Packet& request, Cast::Network::Session& session)
        {
            Common::Network::Packet response;
            response.setTcpHeader(session.getId(), Common::Enums::NO_ENCRYPTION);
            response.setOrder(501);
            response.setExtra(32);
            session.asyncWrite(response);
        }
    }
}

#endif