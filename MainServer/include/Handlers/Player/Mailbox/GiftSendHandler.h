#ifndef MAILBOX_GIFT_SEND_HANDLER_H
#define MAILBOX_GIFT_SEND_HANDLER_H

#include "../../../Network/MainSession.h"
#include "../../../Network/MainSessionManager.h"
#include "Network/Packet.h"
#include "../../../MainEnums.h"
#include "../../../Structures/Mailbox.h"

#include <algorithm>
#include <cstring>

namespace Main
{
	namespace Handlers
	{
		inline void handleMailboxGiftSend(const Common::Network::Packet& request, Main::Network::Session& session)
		{
			// todo
		}
	}
}

#endif