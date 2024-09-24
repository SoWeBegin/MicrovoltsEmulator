#ifndef BLOCK_PLAYER_HANDLER_H
#define BLOCK_PLAYER_HANDLER_H

#include "../../../Network/MainSession.h"
#include "../../../Network/MainSessionManager.h"
#include "../../../Persistence/MainDatabaseManager.h"
#include "Network/Packet.h"

namespace Main
{
	namespace Handlers
	{
		inline void handlePlayerBlock(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager,
			Main::Persistence::PersistentDatabase& database)
		{
			if (request.getOption() == 2) // for some reason the client always sends this option for blocked players list
			{
				char targetAccountName[16]{};
				std::memcpy(targetAccountName, request.getData(), sizeof(targetAccountName));

				Common::Network::Packet response;
				response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
				response.setCommand(request.getOrder(), 0, 1, 0);

				if (auto* targetSession = sessionsManager.findSessionByName(targetAccountName))
				{
					const std::uint32_t targetAccountId = targetSession->getAccountInfo().accountID;
					session.blockAccount(targetAccountId, targetAccountName);
					session.deleteFriend(targetAccountId);
					targetSession->deleteFriend(session.getAccountInfo().accountID, false);
					session.asyncWrite(response);
				}
				else
				{
					const std::uint32_t targetAccountId = database.blockPlayerByNickname(session.getAccountInfo().accountID, targetAccountName);
					if (targetAccountId != -1)
					{
						session.blockAccount(targetAccountId, targetAccountName);
						session.deleteFriend(targetAccountId);
						session.asyncWrite(response);
					}
				}
			}
		}
	}
}

#endif
