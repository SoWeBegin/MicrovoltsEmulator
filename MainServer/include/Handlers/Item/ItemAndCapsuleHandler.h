#ifndef ITEM_AND_CAPSULE_SPIN_HANDLER_H
#define ITEM_AND_CAPSULE_SPIN_HANDLER_H

#include "../../../include/Network/MainSession.h"
#include "../../../include/Network/MainSessionManager.h"
#include "Network/Packet.h"
#include "../../Structures/PlayerLists/Friend.h"
#include "DeleteItemHandler.h"
#include "../../Structures/Item/SpawnedItem.h"
#include "../../Utilities.h"
#include "../../Boxes/BoxBase.h"
#include "../CapsuleSpinHandler.h"

namespace Main
{
	namespace Handlers
	{
		inline void handleGeneralItem(const Common::Network::Packet& request, Main::Network::Session& session, Main::Persistence::PersistentDatabase& db,
			const std::unordered_map<std::uint32_t, std::unique_ptr<Main::Box::IBox>>& m_boxes, const Main::Structures::ItemSerialInfo& openedItemSerialInfo,
			Classes::CapsuleManager capsuleManager, Network::SessionsManager sessionsManager)
		{
			if (request.getMission() == 2) // capsule spin
			{
				Main::Handlers::handleCapsuleSpin(request, session, capsuleManager, sessionsManager);
			}
			else
			{
				const auto& userItems = session.getItems();

				auto response = request;

				if (userItems.contains(openedItemSerialInfo.itemNumber))
				{
					const auto& item = userItems.at(openedItemSerialInfo.itemNumber);
					if (m_boxes.contains(item.id))
					{
						if (item.id >= Main::Enums::MP_100 && item.id <= Main::Enums::MP_500000)
						{ // mp box
							response.setMission(1);
							session.asyncWrite(response);
							session.sendMp(m_boxes.at(item.id)->get());
						}
						else
						{ // normal box
							Main::Structures::BoxItem boxItem;
							Main::Structures::ItemSerialInfo wonItemSerialInfo;
							wonItemSerialInfo.itemNumber = session.getLatestItemNumber() + 1;
							session.setLatestItemNumber(wonItemSerialInfo.itemNumber);
							boxItem.itemId = m_boxes.at(item.id)->get();
							boxItem.serialInfo = wonItemSerialInfo;

							response.setOption(1); 
							response.setData(reinterpret_cast<std::uint8_t*>(&boxItem), sizeof(boxItem));
							session.asyncWrite(response);
							session.deleteItem(openedItemSerialInfo); // Delete the box / item that was just opened
							session.addItem(boxItem);
						}
					}
					else
					{
						const std::uint32_t accountID = session.getAccountInfo().accountID;
						auto execDbQuery = [&](auto dbAction) {
							if (dbAction()) Details::sendMessage("relog.", session);
							else Details::sendMessage("error, retry.", session);
							};

						Main::Structures::ItemSerialInfo wonItemSerialInfo;
						wonItemSerialInfo.itemNumber = session.getLatestItemNumber() + 1;
						response.setOption(1);
						Main::Structures::BoxItem boxItem;

						switch (item.id)
						{
						case Main::Enums::KILLDEATH_RESET: 
							execDbQuery([&]() { return db.resetKillDeath(accountID); });
							break;

						case Main::Enums::RECORD_RESET: 
							execDbQuery([&]() { return db.resetRecord(accountID); });
							break;

						case Main::Enums::BATTERY_500_MP: case Main::Enums::BATTERY_500_RT:
							session.sendBattery(500);
							Details::sendMessage("relog.", session);
							break;

						case Main::Enums::BATTERY_1000_MP: case Main::Enums::BATTERY_1000_RT:
							session.sendBattery(1000);
							Details::sendMessage("relog.", session);
							break;

						case Main::Enums::BATTERY_EXPANSION: 
							execDbQuery([&]() { return db.batteryExpansion(accountID); });
							break;

						default:
							Details::sendMessage("Unknown item: " + std::to_string(item.id), session);
							return; // if error, don't delete the item from the inventory
						}
					}
					session.deleteItem(openedItemSerialInfo);
				}
			}
		}

	}
}

#endif