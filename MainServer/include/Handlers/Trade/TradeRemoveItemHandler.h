#ifndef TRADE_REMOVE_ITEM_HANDLER_H
#define TRADE_REMOVE_ITEM_HANDLER_H

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
		inline void handleTradeItemRemoval(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager)
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
				Main::Structures::TradeAddedItemDetailed tradeItem{ session.getAccountInfo().accountID, itemSerialInfo, itemID };			
				response.setExtra(Enums::TradeSystemExtra::TRADE_SUCCESS);
				session.removeTradedItem(itemSerialInfo);
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