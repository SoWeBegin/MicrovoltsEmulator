#ifndef FRIEND_DELETE_HANDLER_H
#define FRIEND_DELETE_HANDLER_H

#include "../../../Network/MainSession.h"
#include "../../../Network/MainSessionManager.h"
#include "Network/Packet.h"

namespace Main
{
	namespace Handlers
	{
		inline void handleFriendDeletion(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager)
		{
			std::uint32_t targetAccountIdToDelete;
			std::memcpy(&targetAccountIdToDelete, request.getData(), sizeof(std::uint32_t));

			session.deleteFriend(targetAccountIdToDelete);

			if (auto* foundSession = sessionsManager.getSessionByAccountId(targetAccountIdToDelete))
			{
				foundSession->deleteFriend(session.getAccountInfo().accountID, false);
			}
			else
			{
				// todo
			}
		}
	}
}

#endif