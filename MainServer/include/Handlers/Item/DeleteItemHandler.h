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
			std::uint32_t totalItemsToDelete;
			std::memcpy(&totalItemsToDelete, request.getData(), sizeof(totalItemsToDelete)); 
			for (std::uint32_t i = 0; i < totalItemsToDelete; ++i)
			{
				std::memcpy(&itemSerialInfoToDelete, request.getData() + 4 * (i+1), sizeof(itemSerialInfoToDelete));
				session.deleteItem(itemSerialInfoToDelete);
			}
		}
	}
}

#endif