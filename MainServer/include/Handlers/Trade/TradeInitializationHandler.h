#ifndef TRADE_REQUEST_HANDLER_H
#define TRADE_REQUEST_HANDLER_H

#include "../../../include/Network/MainSession.h"
#include "../../../include/Network/MainSessionManager.h"
#include "../../../include/MainEnums.h"
#include "../../../include/Structures/TradeSystem/TradePlayerInfo.h"
#include "Network/Packet.h"
#include <vector>
#include <cstdint>

namespace Main
{
	namespace Handlers
	{
		inline void sendInitialTradeInfo(Main::Network::Session& session, Main::Network::Session& targetSession, Common::Network::Packet& response)
		{
			const auto& selfAccountInfo = session.getAccountInfo();
			const auto& selfEquippedItems = session.getEquippedItems();
			std::uint32_t selfEquippedHair = 0;
			std::uint32_t selfEquippedEyes = 0;

			// Todo: Probably identical items for different characters are differentiated by type (maybe naomi 0, amelia 25, etc?)
			if (selfEquippedItems.contains(0)) selfEquippedHair = selfEquippedItems.at(0).id;
			else if (selfEquippedItems.contains(25)) selfEquippedHair = selfEquippedItems.at(25).id;
			if (selfEquippedItems.contains(1)) selfEquippedEyes = selfEquippedItems.at(1).id;

			Main::Structures::TradePlayerInfo tradePlayerInfo{ selfAccountInfo.accountID, static_cast<std::uint32_t>(selfAccountInfo.latestSelectedCharacter), selfEquippedHair, selfEquippedEyes };
			response.setData(reinterpret_cast<std::uint8_t*>(&tradePlayerInfo), sizeof(tradePlayerInfo));
			targetSession.asyncWrite(response);

			targetSession.setCurrentlyTradingWithAccountId(selfAccountInfo.accountID);
		}

		inline void handleTradeInitialization(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager)
		{
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());
			response.setExtra(request.getExtra());

			std::uint32_t targetAccountId;
			std::memcpy(&targetAccountId, request.getData() + sizeof(std::uint32_t), sizeof(std::uint32_t));

			if (auto* targetSession = sessionsManager.getSessionByAccountId(targetAccountId))
			{
				const auto targetPlayerState = targetSession->getPlayerState();
				if (!session.isFriend(targetAccountId))
				{
					response.setExtra(Enums::TradeSystemExtra::PLAYERS_NOT_FRIENDS);
					session.asyncWrite(response);
				}
				else if (request.getExtra() == Enums::TradeSystemExtra::TRADE_DECLINED)
				{
					response.setExtra(Enums::TradeSystemExtra::TRADE_DECLINED);
					sendInitialTradeInfo(session, *targetSession, response);
				}
				else if (targetPlayerState == Common::Enums::PlayerState::STATE_LOBBY || targetPlayerState == Common::Enums::PlayerState::STATE_NORMAL)
				{
					response.setExtra(Enums::TradeSystemExtra::TRADE_SUCCESS);
					session.setPlayerState(Common::Enums::STATE_TRADE);
					targetSession->setPlayerState(Common::Enums::STATE_TRADE);
					session.temporarilySealAllItems();
					targetSession->temporarilySealAllItems();
					sendInitialTradeInfo(*targetSession, session, response);
					sendInitialTradeInfo(session, *targetSession, response);
				}
				else
				{
					response.setExtra(Enums::TradeSystemExtra::CANNOT_TRADE_NOW_OR_PLAYER_OFFLINE);
					session.asyncWrite(response);
				}
			}
			
		}
	}
}

#endif