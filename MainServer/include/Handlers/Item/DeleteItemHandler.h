#ifndef DELETE_ITEM_HANDLER_H
#define DELETE_ITEM_HANDLER_H

#include "../../Network/MainSession.h"
#include "../../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "Network/Packet.h"

namespace Main
{
	namespace Handlers
	{
		inline void handleItemDelete(const Common::Network::Packet& request, Main::Network::Session& session)
		{
			Main::Structures::ItemSerialInfo itemSerialInfoToDelete{};
			std::memcpy(&itemSerialInfoToDelete, request.getData(), sizeof(itemSerialInfoToDelete));
			session.deleteItem(itemSerialInfoToDelete);
		}
	}
}

#endif