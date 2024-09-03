#ifndef TRADE_FINALIZE_HANDLER_H
#define TRADE_FINALIZE_HANDLER_H

#include "../../../include/Network/MainSession.h"
#include "../../../include/Network/MainSessionManager.h"
#include "Enums/GameEnums.h"
#include "Network/Packet.h"
#include <vector>
#include <cstdint>
#include <optional>
#include <array>
#include "../../Structures/TradeSystem/TradeAck.h"


namespace Main
{
	namespace Handlers
	{
#pragma pack(push)
		struct TradeUnusedFinalItem
		{
			char unused[8]{};
			std::uint32_t totalNewMp{}; // This was used in the old trade system
			std::uint32_t unusedTotal{};
			std::array<Main::Structures::ItemSerialInfo, 10> itemSerialInfosUnused{};
			std::uint32_t unusedTotal1{};
			std::array<std::uint32_t, 10> itemIdsUnused{};
		};
#pragma pack(pop)

		// Currently: Both players need to relog to see their newly obtained items. However, after a trade their traded items are directly removed from their inventory.
		inline void handleTradeFinalization(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager)
		{
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());
			if (auto* targetSession = sessionsManager.getSessionByAccountId(session.getCurrentlyTradingWithAccountId()))
			{
				if (targetSession->hasPlayerLocked())
				{
					session.setPlayerState(Common::Enums::PlayerState::STATE_INVENTORY);
					targetSession->setPlayerState(Common::Enums::PlayerState::STATE_INVENTORY);
					// The other player has confirmed the trade, we can proceed
					response.setOrder(199);

					auto selfTradedItems = session.getTradedItems();
					auto targetTradedItems = targetSession->getTradedItems();
					std::uint64_t targetLatestItemNumber = targetSession->getLatestItemNumber();
					std::uint64_t selfLatestItemNumber = session.getLatestItemNumber();

					// Self
					TradeUnusedFinalItem unusedFinalTradeItem;
					session.setAccountMicroPoints(session.getAccountInfo().microPoints - 5000 * session.getTradedItems().size());
					session.sendCurrency();
					unusedFinalTradeItem.totalNewMp = session.getAccountInfo().microPoints;
					response.setData(reinterpret_cast<std::uint8_t*>(&unusedFinalTradeItem), sizeof(unusedFinalTradeItem));
					for (auto& current : selfTradedItems)
					{
						current.itemSerialInfo.itemNumber = ++targetLatestItemNumber;
					}
					targetSession->setLatestItemNumber(targetLatestItemNumber);
					session.asyncWrite(response);

					// Target
					targetSession->setAccountMicroPoints(targetSession->getAccountInfo().microPoints - 5000 * targetSession->getTradedItems().size());
					targetSession->sendCurrency();
					unusedFinalTradeItem.totalNewMp = targetSession->getAccountInfo().microPoints;
					response.setData(reinterpret_cast<std::uint8_t*>(&unusedFinalTradeItem), sizeof(unusedFinalTradeItem));
					for (auto& current : targetTradedItems)
					{
						current.itemSerialInfo.itemNumber = ++selfLatestItemNumber;
					}
					session.setLatestItemNumber(selfLatestItemNumber);
					targetSession->asyncWrite(response);
					session.deleteItems(session.getTradedItems()); // Might fail! how to recover?!
					session.resetTradeInfo();
					targetSession->deleteItems(targetSession->getTradedItems());
					targetSession->spawnItems(selfTradedItems);
					session.spawnItems(targetTradedItems);
					targetSession->resetTradeInfo();

				}
				else if (session.hasPlayerLocked())
				{
					// The player has already clicked "Confirm", but tries to click "Confirm" again ==> Tell them that both players must confirm in this case
					response.setExtra(Main::Enums::TradeSystemExtra::BOTH_PLAYERS_MUST_CONFIRM_TRADE_BEFORE_FINALIZATION);
					Main::Structures::TradeAck ack{ Main::Structures::UniqueId{}, session.getAccountInfo().accountID };
					response.setData(reinterpret_cast<std::uint8_t*>(&ack), sizeof(ack));
					session.asyncWrite(response);
					return;
				}
				else
				{
					// The other player has not yet confirmed the trade and neither did we, notify the other player that we've confirmed the trade now
					session.lockTrade();
					response.setOrder(197);
					response.setExtra(Main::Enums::TradeSystemExtra::TRADE_CONFIRMED_NOTIFY_OTHER_PLAYER);
					targetSession->asyncWrite(response);
				}
			}
			else
			{
				response.setOrder(198);
				response.setExtra(Enums::TradeSystemExtra::CANNOT_TRADE_NOW_OR_PLAYER_OFFLINE);
				session.asyncWrite(response);
				session.resetTradeInfo();
				session.setPlayerState(Common::Enums::PlayerState::STATE_INVENTORY);
			}
		}
	}
}

#endif