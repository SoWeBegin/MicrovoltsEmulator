#ifndef MAILBOX_DELETE_HANDLER_H
#define MAILBOX_DELETE_HANDLER_H

#include "../../../Network/MainSession.h"
#include "../../../../include/MainEnums.h"
#include "Network/Packet.h"

namespace Main
{
	namespace Handlers
	{
		// TODO: Implement "mass" mailbox delete (= deleting multiple mailboxes at once), unsure if required by the client
		inline void handleMailboxDelete(const Common::Network::Packet& request, Main::Network::Session& session)
		{
			std::uint32_t timestamp;
			std::memcpy(&timestamp, request.getData() + 4, sizeof(timestamp));
			
			if (request.getMission() == Main::Enums::MailboxMission::MISSION_MAILBOX_RECEIVED)
			{
				session.deleteReceivedMailbox(timestamp);
			}
			else
			{
				session.deleteSentMailbox(timestamp);
			}
		}
	}
}

#endif