#ifndef MAILBOX_DELETE_HANDLER_H
#define MAILBOX_DELETE_HANDLER_H

#include "../../../Network/MainSession.h"
#include "Network/Packet.h"
#include "../../../../include/MainEnums.h"

namespace Main
{
	namespace Handlers
	{
		// TODO: Implement "mass" mailbox delete (= deleting multiple mailboxes at once)
		inline void handleMailboxDelete(const Common::Network::Packet& request, Main::Network::Session& session)
		{
			Main::Structures::UniqueId uniqueId;
			std::uint32_t accountId, timestamp;
			std::memcpy(&accountId, request.getData() + 4, sizeof(accountId));
			std::memcpy(&timestamp, request.getData() + 8, sizeof(timestamp));
			
			if (request.getMission() == Main::Enums::MailboxMission::MISSION_MAILBOX_RECEIVED)
			{
				session.deleteReceivedMailbox(timestamp, accountId);
			}
			else
			{
				session.deleteSentMailbox(timestamp, accountId);
			}
		}
	}
}

#endif