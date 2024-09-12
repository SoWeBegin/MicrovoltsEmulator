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

			handleMapEvents(request, session, database);

			handleAccountInformation(request, session, sessionsManager, database, accountInfo, timeSinceLastServerRestart);

			handleInventoryInformation(request, session, sessionsManager, database);

			handleAccountInformation(request, session, sessionsManager, database, accountInfo, timeSinceLastServerRestart, 59);

			// Initial game logo
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(204);
			struct Advertisement
			{
				std::uint32_t unknown = 20;
				char fileName[17]{"welcome_logo.dds"};
				char unused[495];
			} advertisement;
			response.setData(reinterpret_cast<std::uint8_t*>(&advertisement), sizeof(advertisement));
			session.asyncWrite(response);

			// Weekly rewards (in current client it's VIP rewards) => works, elaborate further
			/*
			// Set weekly rewards (in this client it's VIP rewards)
			response.setOrder(182);
			response.setExtra(28);
			response.setOption(2);
			response.setMission(1);
			struct WeeklyReward
			{
				std::uint64_t unknown = 450971893761;//maybe day, idk?
				std::array<std::uint32_t, 7> weeklyItems{}; 

				WeeklyReward()
				{
					weeklyItems.fill(4306001); // change this to MP box
				}
			};
			WeeklyReward weeklyReward;
			response.setData(reinterpret_cast<std::uint8_t*>(&weeklyReward), sizeof(weeklyReward));
		    session.asyncWrite(response);

			// Show them
			response.setOrder(66);
			response.setExtra(051);
			response.setOption(2);
			response.setMission(0);
			// n.b. option0, mission3 => story reward
			Main::Structures::SpawnedItem spawnedItem{};
			spawnedItem.itemId = 4306001; // change this to mp box
			response.setData(reinterpret_cast<std::uint8_t*>(&spawnedItem), sizeof(spawnedItem));
			session.asyncWrite(response);
			spawnedItem.serialInfo.itemNumber = session.getLatestItemNumber() + 1;
			session.addItem(spawnedItem);
			session.setLatestItemNumber(spawnedItem.serialInfo.itemNumber);
			*/
		}
	}
}


#endif	