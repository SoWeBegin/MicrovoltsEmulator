#ifndef MAILBOX_HANDLER_H
#define MAILBOX_HANDLER_H

#include "../../../Network/MainSession.h"
#include "../../../Network/MainSessionManager.h"
#include "Network/Packet.h"
#include "../../../MainEnums.h"
#include "../../../Structures/Mailbox.h"

#include <algorithm>
#include <cstring>
#include <cstdint>

namespace Main
{
	namespace Handlers
	{
		inline void handleMailboxCommunication(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager,
			Main::Persistence::PersistentDatabase& database, const Main::ClientData::MailboxMessage& mailboxData)
		{
			const auto& accountInfo = session.getAccountInfo();
			constexpr const std::uint32_t maxMailboxes = 100;
			constexpr const std::uint32_t nicknameSize = 16;

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());

			if (request.getOption() == 2) // option seems to always be 2
			{
				if (session.getMailboxSent().size() > maxMailboxes)
				{
					response.setExtra(Main::Enums::MailboxExtra::SENDER_NO_SPACE_LEFT);
					session.asyncWrite(response);
					return;
				}

				
				auto* targetSession = sessionsManager.findSessionByName(mailboxData.nickname);
				if (!targetSession)
				{
					Main::Structures::Mailbox mailbox;
					mailbox.timestamp = static_cast<__time32_t>(std::time(0));
					std::memcpy(mailbox.nickname, mailboxData.nickname, nicknameSize);
					std::memcpy(mailbox.message, mailboxData.message, request.getDataSize() - nicknameSize);
					auto res = database.storeOfflineMailbox(mailbox, accountInfo.nickname);

					if (!res)
					{
						response.setExtra(Enums::MailboxExtra::RECEIVER_NOT_FOUND);
						session.asyncWrite(response);
						return;
					}
					else
					{
						mailbox.accountId = accountInfo.accountID;
						std::memcpy(mailbox.nickname, mailboxData.nickname, nicknameSize);
						session.addMailboxSent(mailbox);
					}
				}
				else
				{
					const auto& targetAccountInfo = targetSession->getAccountInfo();
					// The target is online, use their cached information
					
					if (targetSession->getMailboxReceived().size() > maxMailboxes)
					{
						response.setExtra(Main::Enums::MailboxExtra::RECEIVER_NO_SPACE_LEFT);
						session.asyncWrite(response);
						return;
					}
					else if (targetSession->hasBlocked(accountInfo.accountID))
					{
						response.setExtra(Main::Enums::MailboxExtra::MAILBOX_RECEIVER_BLOCKED_SENDER);
						session.asyncWrite(response);
						return;
					}
					else // send the mailbox to the target
					{
						response.setExtra(Main::Enums::MailboxExtra::MAILBOX_RECEIVED);
						response.setData(reinterpret_cast<std::uint8_t*>(const_cast<char*>(accountInfo.nickname)), 16);
						targetSession->asyncWrite(response);

						Main::Structures::Mailbox mailbox{ targetAccountInfo.accountID, static_cast<__time32_t>(std::time(0)) };
						constexpr std::size_t nicknameSize = 16;
						std::memcpy(mailbox.nickname, accountInfo.nickname, nicknameSize);
						std::memcpy(mailbox.message, mailboxData.message, request.getDataSize() - nicknameSize);
						targetSession->addMailboxReceived(mailbox);

						mailbox.accountId = accountInfo.accountID;
						std::memcpy(mailbox.nickname, targetSession->getAccountInfo().nickname, nicknameSize);
						session.addMailboxSent(mailbox);
					}
				}
			}
		}
	}
}

#endif
