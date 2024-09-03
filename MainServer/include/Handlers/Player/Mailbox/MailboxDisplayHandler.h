#ifndef MAILBOX_DISPLAYER_HANDLER_H
#define MAILBOX_DISPLAYER_HANDLER_H

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
		inline void handleMailboxDisplay(const Common::Network::Packet& request, Main::Network::Session& session)
		{
			// Accidentally deleted everything here, reimplement 
		}
	}
}

#endif