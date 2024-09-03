
#ifndef BOUGHT_ITEM_HANDLER_H
#define BOUGHT_ITEM_HANDLER_H

#include <unordered_map>
#include "../../../include/Network/MainSession.h"
#include "../../Structures/Item/MainBoughtItem.h"
#include "../../CdbUtils.h"
#include "../../MainEnums.h"
#include "../../Structures/Item/MainItem.h"
#include "../../Structures/AccountInfo/MainAccountInfo.h"

namespace Main
{
	namespace Handlers
	{
		inline void removeAmountByCurrencyType(Main::Enums::ItemCurrencyType currencyType, std::uint32_t amountToRemove, Main::Network::Session& session,
			const Main::Structures::AccountInfo& accountInfo)
		{
			if (Main::Enums::ITEM_MP == currencyType)
			{
				session.setAccountMicroPoints(accountInfo.microPoints - amountToRemove);
				return;
			}
			else if (Main::Enums::ITEM_RT == currencyType)
			{
				session.setAccountRockTotens(accountInfo.rockTotens - amountToRemove);
				return;
			}
			// currently coins/coupons aren't handled when buying items (i.e. coin shop is not implemented yet)
			return;
		}

		// TODO: Remove code duplication
		inline void handleBoughtItem(const Common::Network::Packet& request, Main::Network::Session& session)
		{
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());
			response.setOption(request.getOption()); // number of items bought

			if (request.getMission() == 1) // Item is expired and the user wants to renew it
			{
				response.setMission(request.getMission());
				Main::Structures::ItemSerialInfo itemSerialInfoToProlong{};
				std::vector<Main::Structures::BoughtItemToProlong> itemsToProlong;
				std::unordered_map<Main::Enums::ItemCurrencyType, std::uint32_t> totalCurrencySpentByType;
				for (std::uint32_t idx = 0; idx < request.getOption(); ++idx)
				{
					std::memcpy(&itemSerialInfoToProlong, request.getData() + idx * 8, sizeof(itemSerialInfoToProlong));
					auto item = session.findItemBySerialInfo(itemSerialInfoToProlong);
					if (item == std::nullopt) continue;
					const Main::ConstantDatabase::CdbUtil cdb(item->id);
					const auto amountByCurrencyType = cdb.getItemPrice();
					totalCurrencySpentByType[amountByCurrencyType->first] += amountByCurrencyType->second;
					Main::Structures::BoughtItemToProlong boughtItemToProlong{};
					boughtItemToProlong.serialInfo = itemSerialInfoToProlong;
					itemsToProlong.push_back(boughtItemToProlong);
					session.replaceItem(item->serialInfo.itemNumber, Main::Structures::BoughtItemToProlong{}.serialInfo, *(cdb.getItemDuration()));
				}

				const auto& accountInfo = session.getAccountInfo();
				const std::size_t sessionId = session.getId();
				for (const auto& [currencyType, totalSpent] : totalCurrencySpentByType)
				{
					if (Main::Enums::ITEM_MP == currencyType && totalSpent > accountInfo.microPoints || Main::Enums::ITEM_RT == currencyType && totalSpent > accountInfo.rockTotens)
					{
						return;
					}
				}

				response.setData(reinterpret_cast<std::uint8_t*>(itemsToProlong.data()), itemsToProlong.size() * sizeof(Main::Structures::BoughtItemToProlong));
				session.asyncWrite(response);

				for (const auto& [currencyType, totalSpent] : totalCurrencySpentByType)
				{
					removeAmountByCurrencyType(currencyType, totalSpent, session, accountInfo);
				}
			}
			else // Buy new item
			{
				if (!session.hasEnoughInventorySpace(request.getOption())) return;

				std::uint64_t latestItemNumber = session.getLatestItemNumber();
				std::vector<Main::Structures::BoughtItem> boughtItems;
				std::unordered_map<Main::Enums::ItemCurrencyType, std::uint32_t> totalCurrencySpentByType;
				for (std::uint32_t idx = 0; idx < request.getOption(); ++idx)
				{
					std::uint32_t itemId;
					std::memcpy(&itemId, request.getData() + idx * 4, sizeof(std::uint32_t));

					Main::Structures::BoughtItem boughtItem(itemId);
					const Main::ConstantDatabase::CdbUtil cdb(boughtItem.itemId);
					const auto amountByCurrencyType = cdb.getItemPrice();
					totalCurrencySpentByType[amountByCurrencyType->first] += amountByCurrencyType->second;
					boughtItem.serialInfo.itemNumber = ++latestItemNumber;
					boughtItems.push_back(boughtItem);
				}

				// Check if the player has enough money for all items
				const auto& accountInfo = session.getAccountInfo();
				const std::size_t sessionId = session.getId();
				for (const auto& [currencyType, totalSpent] : totalCurrencySpentByType)
				{
					if (Main::Enums::ITEM_MP == currencyType && totalSpent > accountInfo.microPoints || Main::Enums::ITEM_RT == currencyType && totalSpent > accountInfo.rockTotens)
					{
						return;
					}
				}

				response.setData(reinterpret_cast<std::uint8_t*>(boughtItems.data()), boughtItems.size() * sizeof(Main::Structures::BoughtItem));
				session.asyncWrite(response);

				session.setLatestItemNumber(latestItemNumber);

				// TODO: add item and remove currency must be atomic!!!
				session.addItems(boughtItems);
				for (const auto& [currencyType, totalSpent] : totalCurrencySpentByType)
				{
					removeAmountByCurrencyType(currencyType, totalSpent, session, accountInfo);
				}
			}
		}
	}
}

#endif
