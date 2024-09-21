#ifndef MAIN_INITIALPLAYERINFO_HANDLER_H
#define MAIN_INITIALPLAYERINFO_HANDLER_H


#include "Network/Session.h"
#include "MainAccountInfoHandler.h"
#include "MainInventoryHandler.h"
#include "../EventsHandlers.h"
#include "AuthorizationHandler.h"


namespace Main
{
	namespace Handlers
	{
		inline void handleInitialPlayerInfos(const Common::Network::Packet& request, Main::Network::Session& session,
			Main::Network::SessionsManager& sessionsManager, Main::Persistence::PersistentDatabase& database, std::uint64_t timeSinceLastServerRestart)
		{
			Main::Structures::AccountInfo accountInfo = handleAuthorization(request, session, database);

			//handleModeEvents(request, session, database);

			//handleMapEvents(request, session, database);

			handleAccountInformation(request, session, sessionsManager, database, accountInfo, timeSinceLastServerRestart);

			handleInventoryInformation(request, session, sessionsManager, database);
		}
	}
}


#endif	