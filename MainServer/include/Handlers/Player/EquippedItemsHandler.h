#ifndef EQUIPPED_ITEMS_HANDLER
#define EQUIPPED_ITEMS_HANDLER

#include "../../Network/MainSession.h"
#include "Network/Packet.h"
#include "../../MainEnums.h"
#include "../../Structures/Item/MainBoughtItem.h"

namespace Main
{
	namespace Handlers
	{
		inline void handleEquippedItemSwitch(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager)
		{
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());
			response.setExtra(request.getExtra());
			response.setOption(request.getOption()); // Number of new equipped items / equipped items to unequip for the current character

			if (request.getExtra() == 51)
			{
				for (std::size_t idx = 0; idx < request.getOption(); ++idx)
				{
					std::uint16_t character;
					std::uint32_t itemNumber;
					std::memcpy(&character, request.getData() + idx * 12, sizeof(character));
					std::memcpy(&itemNumber, request.getData() + idx * 12 + 4, sizeof(itemNumber));
					session.switchItemEquip(character, itemNumber);
				}
			}
			else if (request.getMission() == 1)
			{
				// Remove a single item based on the passed extra, which is the EquippedItem's type
				session.unequipItem(request.getExtra());
				session.asyncWrite(response);
				return;
			}
			else if (request.getMission() == 0)
			{
				constexpr std::uint16_t MAX_TYPE = 30;
				for (std::size_t idx = 0; idx < request.getOption(); ++idx)
				{
					Main::Structures::ItemSerialInfo itemSerialInfo;
					std::uint64_t val = 0;
					std::memcpy(&itemSerialInfo, request.getData() + idx * 8, sizeof(itemSerialInfo));
					std::memcpy(&val, request.getData() + idx * 8, sizeof(itemSerialInfo));
					if (val < static_cast<std::uint64_t>(MAX_TYPE))
					{
						std::cout << "Unequipping items in handleEquippedItemSwitch()\n";
						// Item must be removed, with val == equippedItem.type
						session.unequipItem(val);
					}
					else
					{
						session.equipItem(itemSerialInfo.itemNumber);
					}
				}
			}
			session.asyncWrite(response);
		}
	}
}

#endif