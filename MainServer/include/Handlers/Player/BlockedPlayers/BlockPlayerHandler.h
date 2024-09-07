#ifndef BLOCK_PLAYER_HANDLER_H
#define BLOCK_PLAYER_HANDLER_H

#include "../../../Network/MainSession.h"
#include "../../../Network/MainSessionManager.h"
#include "Network/Packet.h"

namespace Main
{
	namespace Handlers
	{
		inline void handlePlayerBlock(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager)
		{
			char targetAccountName[16]{};
			std::memcpy(targetAccountName, request.getData(), sizeof(targetAccountName));

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());
			
			if (request.getOption() == 2) // for some reason the client always sends this option for blocked players list
			{
				if (auto* targetSession = sessionsManager.findSessionByName(targetAccountName)) // player is online
				{
					const std::uint32_t targetAccountId = targetSession->getAccountInfo().accountID;
					session.blockAccount(targetAccountId, targetAccountName);
					response.setExtra(1);
					session.deleteFriend(targetAccountId);
					targetSession->deleteFriend(session.getAccountInfo().accountID, false);
					session.asyncWrite(response);
				}
				else
				{
					// Player is offline, check if the nickname is found in the database, otherwise return extra 6?
				}
			}
		}
	}
}

#endif
