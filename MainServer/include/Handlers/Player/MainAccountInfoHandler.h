#ifndef ACCOUNT_INFO_HANDLER_MAIN_H
#define ACCOUNT_INFO_HANDLER_MAIN_H

#include "Network/Session.h"
#include "Network/Packet.h"
#include "../../MainEnums.h"
#include "../../Structures/AccountInfo/MainAccountInfo.h"
#include "../../../include/Network/MainSession.h"
#include "../../../include/Network/MainSessionManager.h"


namespace Main
{
	namespace Handlers
	{
		inline void handleAccountInformation(const Common::Network::Packet& request, Main::Network::Session& session,
			Main::Network::SessionsManager& sessionsManager, Main::Persistence::PersistentDatabase& database, Main::Structures::AccountInfo& accountInfo, 
			std::uint64_t timeSinceLastServerRestart, std::uint32_t serverId)
		{
			accountInfo.uniqueId.session = session.getId();
			accountInfo.uniqueId.server = serverId;
			accountInfo.serverTime = accountInfo.getUtcTimeMs() - timeSinceLastServerRestart;
			session.setAccountInfo(accountInfo);

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setCommand(413, 0, 1, 0);
			response.setData(reinterpret_cast<std::uint8_t*>(&accountInfo), sizeof(accountInfo));
			session.asyncWrite(response);

			sessionsManager.addSession(&session);

			// Load the user's friendlist from the database
			auto friends = database.loadFriends(accountInfo.accountID);
			session.setFriendList(friends);
			for (const auto& currentFriend : friends)
			{
				if (Main::Network::Session* targetSession = sessionsManager.getSessionByAccountId(currentFriend.targetAccountId))
				{
					// This friend is online. Notify them that we're online
					session.updateFriendSession(targetSession);
					targetSession->updateFriendSession(sessionsManager.getSessionByAccountId(accountInfo.accountID));
					targetSession->logFriend(46, accountInfo.accountID);
				}
			}

			session.setBlockedPlayers(database.loadBlockedPlayers(accountInfo.accountID));
			session.setMute(database.isMuted(accountInfo.accountID));
			session.setMailbox(database.loadMailboxes(accountInfo.accountID, true), true);
			session.setMailbox(database.loadMailboxes(accountInfo.accountID, false), false);

			auto newMailboxes = database.getNewMailboxes(accountInfo.accountID);
			if (newMailboxes.empty()) return;
			response.setCommand(100, 0, Main::Enums::MailboxExtra::MAILBOX_RECEIVED, newMailboxes.size());
			response.setData(reinterpret_cast<std::uint8_t*>(newMailboxes.data()), newMailboxes.size() * sizeof(Main::Structures::Mailbox));
			session.asyncWrite(response);
		}
	}
}

#endif
