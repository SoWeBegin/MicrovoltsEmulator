#ifndef AUTH_ACCOUNTBUSY_HANDLER_H
#define AUTH_ACCOUNTBUSY_HANDLER_H

#include "Network/Session.h"

namespace Auth
{
	namespace Handlers
	{
		inline void handleAccountBusyReconnection(const Common::Network::Packet& request, Common::Network::Session& session)
		{
			session.asyncWrite(const_cast<Common::Network::Packet&>(request));
			// Send disconnection request to MainServer
			// Then proceed normally with the login
			//session.closeSocket();
			//handleAuthUserInformation(session);
		}
	}
}
#endif
