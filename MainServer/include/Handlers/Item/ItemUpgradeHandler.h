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

		enum ItemClientExtra
		{
			ADD_ENERGY_REQUEST = 0,
			UPGRADE_REQUEST = 37,
			RESET_REQUEST = 55
		};

		inline void handleItemUpgrade(const Common::Network::Packet& request, Main::Network::Session& session)
		{
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setCommand(request.getOrder(), request.getMission(), 0, request.getOption());

			const auto& accountInfo = session.getAccountInfo();

			if (request.getExtra() == ItemClientExtra::ADD_ENERGY_REQUEST) 
			{
				std::uint32_t usedEnergy;
				Main::Structures::ItemSerialInfo itemSerialInfo{};
				std::memcpy(&usedEnergy, request.getData() + 8, sizeof(std::uint32_t));
				std::memcpy(&itemSerialInfo, request.getData(), sizeof(itemSerialInfo));

				if (usedEnergy > accountInfo.battery) return; // security check, client already prevents this
				response.setExtra(ItemUpgradeExtra::ENERGY_ADD);
				response.setData(reinterpret_cast<std::uint8_t*>(const_cast<std::uint8_t*>(request.getData())), sizeof(usedEnergy) + sizeof(Main::Structures::ItemSerialInfo));
				if (!session.addEnergyToItem(itemSerialInfo, usedEnergy)) return;
				session.asyncWrite(response);
			}
			else if (request.getExtra() == ItemClientExtra::UPGRADE_REQUEST) 
			{
				Main::Structures::ItemSerialInfo itemSerialInfo{};
				std::memcpy(&itemSerialInfo, request.getData(), sizeof(itemSerialInfo));
				std::array<std::uint8_t, 72> unused{};
				auto found = session.findItemBySerialInfo(itemSerialInfo);
				if (found == std::nullopt) return;

				Main::ConstantDatabase::CdbUtil cdbCurrentId(found->id);
				const auto hasParent = cdbCurrentId.hasParentId();
				if (hasParent == std::nullopt) return;

				const std::uint32_t toAdd = *hasParent ? 1 : (request.getMission() * 10 + 1);
				Main::ConstantDatabase::CdbUtil cdbUtil(found->id + toAdd);
				auto mpNeededForUpgrade = cdbUtil.getMpNeededForUpgrade();
				if (mpNeededForUpgrade == std::nullopt) return;

				// Check if user has enough MP for the upgrade.
				if (accountInfo.microPoints < *mpNeededForUpgrade)
				{
					response.setExtra(ItemUpgradeExtra::NOT_ENOUGH_MP_FOR_UPGRADE);
					session.asyncWrite(response);
					return;
				}

				// Current fail rate: 15%
				static std::mt19937 gen(std::random_device{}()); 
				static std::uniform_int_distribution<int> dist(1, 100); 
				if (found == std::nullopt || dist(gen) <= 15)
				{
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
			else if (request.getExtra() == 55)
			{
				// Item Reset
			}
		}
	}
}

#endif