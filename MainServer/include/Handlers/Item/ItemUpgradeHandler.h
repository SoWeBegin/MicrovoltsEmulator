#ifndef ITEM_ENERGY_INSERTION_HANDLER
#define ITEM_ENERGY_INSERTION_HANDLER

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
		enum ItemUpgradeExtra
		{
			UPGRADE_SUCCESS = 1,
			UPGRADE_FAIL = 6, // also 2
			ITEM_MUST_BE_REPAIRED_FIRST = 8,
			NOT_ENOUGH_MP_FOR_UPGRADE = 14,
			ENERGY_ADD = 32,
		};

		inline void handleItemUpgrade(const Common::Network::Packet& request, Main::Network::Session& session)
		{
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());
			response.setOption(request.getOption());
			response.setMission(request.getMission());

			const auto receivedExtra = request.getExtra();
			const auto& accountInfo = session.getAccountInfo();

			if (receivedExtra == 0) // Client ACK: user adds energy to a weapon.
			{
				std::uint32_t usedEnergy;
				std::memcpy(&usedEnergy, request.getData() + 8, sizeof(std::uint32_t));
				Main::Structures::ItemSerialInfo itemSerialInfo{};
				std::memcpy(&itemSerialInfo, request.getData(), sizeof(itemSerialInfo));

				if (usedEnergy > accountInfo.battery) return; // security check, client already prevents this
				response.setExtra(ItemUpgradeExtra::ENERGY_ADD);
				response.setData(reinterpret_cast<std::uint8_t*>(const_cast<std::uint8_t*>(request.getData())), sizeof(usedEnergy) + sizeof(Main::Structures::ItemSerialInfo));
				if (!session.addEnergyToItem(itemSerialInfo, usedEnergy)) return;
				session.asyncWrite(response);
				return;
			}
			else if (receivedExtra == 37) // upgrade
			{
				Main::Structures::ItemSerialInfo itemSerialInfo{};
				std::memcpy(&itemSerialInfo, request.getData(), sizeof(itemSerialInfo));
				std::vector<std::uint8_t> unused(72);
				auto found = session.findItemBySerialInfo(itemSerialInfo);
				if (found == std::nullopt) return;
				std::cout << "ItemSerialInfo found.\n";

				Main::ConstantDatabase::CdbUtil cdbCurrentId(found->id);

				const auto mission = request.getMission();
				const auto hasParent = cdbCurrentId.hasParentId();
				if (hasParent == std::nullopt) return;
				const std::uint32_t toAdd = *hasParent ? 1 : (mission * 10 + 1);
				Main::ConstantDatabase::CdbUtil cdbUtil(found->id + toAdd);
				auto mpNeededForUpgrade = cdbUtil.getMpNeededForUpgrade();
				if (mpNeededForUpgrade == std::nullopt) return;

				// Check if user has enough MP for the upgrade.
				if (accountInfo.microPoints < *mpNeededForUpgrade)
				{
					std::cout << "mp < MpNeeded\n";
					response.setExtra(ItemUpgradeExtra::NOT_ENOUGH_MP_FOR_UPGRADE);
					session.asyncWrite(response);
					return;
				}

				// Current fail rate: 15%
				static std::mt19937 gen(std::random_device{}()); 
				static std::uniform_int_distribution<int> dist(1, 100); 
				int random = dist(gen); 
				if (found == std::nullopt || random <= 15)
				{
					std::cout << "Fail.\n";
					response.setExtra(ItemUpgradeExtra::UPGRADE_FAIL); 
					response.setData(unused.data(), unused.size());
					session.asyncWrite(response);
					session.deleteItem(itemSerialInfo);
					session.spawnItem(found->id, itemSerialInfo); // spawn new item with correct energy (=reset energy)
					return;
				}

				response.setExtra(ItemUpgradeExtra::UPGRADE_SUCCESS);
				response.setData(unused.data(), unused.size());
				session.asyncWrite(response);
				
				// Delete the old item, respawn new (upgraded) one
				session.deleteItem(itemSerialInfo);
				session.spawnItem(found->id + toAdd, itemSerialInfo);
				session.setAccountMicroPoints(accountInfo.microPoints - *mpNeededForUpgrade);
				session.sendCurrency();
			}
			else if (receivedExtra == 55)
			{
				// Item Reset
			}
		}

		inline void handleGeneralItem(const Common::Network::Packet& request, Main::Network::Session& session, Main::Persistence::PersistentDatabase& db,
			const std::unordered_map<std::uint32_t, std::unique_ptr<Main::Box::IBox>>& m_boxes, Classes::CapsuleManager capsuleManager, Network::SessionsManager sessionsManager)
		{
			if (request.getMission() == 2) // capsule spin
			{
				Main::Handlers::handleCapsuleSpin(request, session, capsuleManager, sessionsManager);
			}
			else
			{
				Main::Structures::ItemSerialInfo openedItemSerialInfo;
				std::memcpy(&openedItemSerialInfo, request.getData(), request.getDataSize());
				const auto& userItems = session.getItems();
				const std::uint32_t accountID = session.getAccountInfo().accountID;

				if (userItems.contains(openedItemSerialInfo.itemNumber))
				{
					const auto& item = userItems.at(openedItemSerialInfo.itemNumber);
					if (m_boxes.contains(item.id))
					{
						auto response = request;

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

							response.setOption(1); // total items
							response.setData(reinterpret_cast<std::uint8_t*>(&boxItem), sizeof(boxItem));
							session.asyncWrite(response);
							session.deleteItem(openedItemSerialInfo); // Delete the box / item that was just opened
							session.addItem(boxItem);
						}
					}
					else
					{
						auto execDbQuery = [&](auto dbAction)
							{
								if (dbAction())
								{
									Details::sendMessage("relog.", session);
								}
								else
								{
									Details::sendMessage("error, retry.", session);
								}
							};

						Main::Structures::ItemSerialInfo wonItemSerialInfo;
						wonItemSerialInfo.itemNumber = session.getLatestItemNumber() + 1;
						auto response = request;
						response.setOption(1);
						Main::Structures::BoxItem boxItem;

						switch (item.id)
						{
						case 4303000: // Kill/Death Reset
							execDbQuery([&]() { return db.resetKillDeath(accountID); });
							break;

						case 4302000: // Record reset
							execDbQuery([&]() { return db.resetRecord(accountID); });
							break;

						case 4305005: case 4305009: // Battery Recharge 500
							session.sendBattery(500);
							Details::sendMessage("relog.", session);
							break;

						case 4305006: case 4305010: // Battery Recharge 1000
							session.sendBattery(1000);
							Details::sendMessage("relog.", session);
							break;

						case 4305007: // Battery Expansion
							execDbQuery([&]() { return db.batteryExpansion(accountID); });
							break;

						default:
							Details::sendMessage("Unknown item: " + std::to_string(item.id), session);
							return; // later use return --> if error, don't delete the item from the inventory
						}
					}
					session.deleteItem(openedItemSerialInfo);
				}
			}
		}

	}
}

#endif