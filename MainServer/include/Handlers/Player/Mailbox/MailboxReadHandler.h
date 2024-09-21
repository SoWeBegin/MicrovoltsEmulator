#ifndef MAILBOX_READ_HANDLER_H
#define MAILBOX_READ_HANDLER_H

#include "../../../Network/MainSession.h"
#include "Network/Packet.h"
#include "../../../../include/MainEnums.h"

namespace Main
{
	namespace Handlers
	{
		inline void handleReadMailbox(const Common::Network::Packet& request, Main::Network::Session& session, Main::Persistence::PersistentDatabase& database)
		{
			std::uint32_t timestamp;
			std::memcpy(&timestamp, request.getData() + 4, sizeof(timestamp));

			if (request.getExtra() == Main::Enums::MailboxExtra::MAILBOX_HAS_BEEN_READ)
			{
				database.updateReadMailbox(session.getAccountInfo().accountID, timestamp);
				auto response = request;
				session.asyncWrite(response);
			}
		}
	}
}

#endif