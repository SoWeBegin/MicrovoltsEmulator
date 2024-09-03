#ifndef TRADE_ADD_ITEM_HANDLER_H
#define TRADE_ADD_ITEM_HANDLER_H

#include "../../../include/Network/MainSession.h"
#include "../../../include/Network/MainSessionManager.h"
#include "../../../include/MainEnums.h"
#include "../../../include/Structures/TradeSystem/TradeSystemItem.h"
#include "Enums/GameEnums.h"
#include "Network/Packet.h"
#include <vector>
#include <cstdint>
#include <optional>

namespace Main
{
	namespace Handlers
	{
		inline void handleAddTradeItem(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager)
		{
			Main::Structures::ItemSerialInfo itemSerialInfo;
			std::memcpy(&itemSerialInfo, request.getData() + 8, sizeof(itemSerialInfo));

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());

			std::uint32_t itemID = 0;
			const auto& item = session.findItemBySerialInfo(itemSerialInfo);
			if (item.has_value()) itemID = item->id;
			else return;

			const auto& accountInfo = session.getAccountInfo();
			if (auto* targetSession = sessionsManager.getSessionByAccountId(session.getCurrentlyTradingWithAccountId()))
			{
				Main::Structures::TradeAddedItemDetailed tradeItem{ session.getAccountInfo().accountID, itemSerialInfo, itemID};
				const auto& targetAccountInfo = targetSession->getAccountInfo();
				const std::size_t targetTotalItems = targetSession->getEquippedItems().size() + targetSession->getItems().size() + session.getTradedItems().size();
				if (targetAccountInfo.inventorySpace < targetTotalItems + 1)
				{
					response.setExtra(Enums::TradeSystemExtra::TARGET_NOT_ENOUGH_INVENTORY_SPACE);
					tradeItem.originalItemOwnerAccountId = targetAccountInfo.accountID;
				}
				else if (session.getTradedItems().size() > 10)
				{
					response.setExtra(Enums::TradeSystemExtra::MAX_NUM_OF_ITEMS_AT_ONCE_REACHED);
				}
				else if (accountInfo.microPoints < (5000 * (session.getTradedItems().size() + 1)))
				{
					std::vector<std::uint8_t> error(8);
					std::memcpy(error.data() + 4, &accountInfo.accountID, sizeof(accountInfo.accountID));
					response.setOrder(193);
					response.setExtra(14);
					response.setMission(0);
					response.setOption(0);
					response.setData(error.data(), error.size());
					session.asyncWrite(response);
					return;
				}
				else
				{
					response.setExtra(Enums::TradeSystemExtra::TRADE_SUCCESS);
					session.addTradedItem(itemID, itemSerialInfo);
				}
				response.setData(reinterpret_cast<std::uint8_t*>(&tradeItem), sizeof(tradeItem));
				session.asyncWrite(response);
				targetSession->asyncWrite(response);
			}
			else
			{
				response.setExtra(Enums::TradeSystemExtra::CANNOT_TRADE_NOW_OR_PLAYER_OFFLINE);
				session.asyncWrite(response);
				session.resetTradeInfo();
				session.setPlayerState(Common::Enums::PlayerState::STATE_INVENTORY);
			}
		}
	}
}

#endif