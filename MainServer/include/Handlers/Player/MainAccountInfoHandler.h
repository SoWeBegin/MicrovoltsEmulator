#ifndef ACCOUNT_INFO_HANDLER_MAIN_H
#define ACCOUNT_INFO_HANDLER_MAIN_H

#include "Network/Session.h"
#include "Network/Packet.h"
#include "../../Structures/AccountInfo/MainAccountInfo.h"
#include "../../MainEnums.h"
#include "Utils/Utils.h"
#include "../../../include/Network/MainSession.h"
#include "../../../include/Network/MainSessionManager.h"


namespace Main
{
	namespace Handlers
	{
		inline void handleAccountInformation(const Common::Network::Packet& request, Main::Network::Session& session,
			Main::Network::SessionsManager& sessionsManager, Main::Persistence::PersistentDatabase& database, Main::Structures::AccountInfo& accountInfo, std::uint32_t extra = 1)
		{
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(413);
			response.setExtra(extra);

			// AccountInfo closure packet
			if (extra == 59)
			{
				std::vector<std::uint8_t> unused(28);
				for (std::size_t i = 0; i < unused.size(); ++i) unused[i] = 0;
				response.setData(unused.data(), 28);
				session.asyncWrite(response);
				return;
			}

			accountInfo.uniqueId.session = session.getId();
			accountInfo.uniqueId.server = 4; // currently hardcoded
		    accountInfo.setServerTime();

			response.setData(reinterpret_cast<std::uint8_t*>(&accountInfo), sizeof(accountInfo));
			session.asyncWrite(response);
			session.setAccountInfo(accountInfo);
			sessionsManager.addSession(&session);

			// Load the user's friendlist from the database
			auto friends = database.loadFriends(accountInfo.accountID);

			// Set the user's friendlist
			session.setFriendList(friends);

			// Find all the online friends and set them
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
			session.setOnlineDatabaseStatus();

			auto newMailboxes = database.getNewMailboxes(accountInfo.accountID);
			if (newMailboxes.empty()) return;
			// Send online messages (mailbox) if there are new ones
			response.setOrder(104);
			response.setOption(newMailboxes.size());
			response.setExtra(Main::Enums::MailboxExtra::MAILBOX_RECEIVED);
			response.setData(reinterpret_cast<std::uint8_t*>(newMailboxes.data()), newMailboxes.size() * sizeof(Main::Structures::Mailbox));
			session.asyncWrite(response);

		}
	}
}

#endif
