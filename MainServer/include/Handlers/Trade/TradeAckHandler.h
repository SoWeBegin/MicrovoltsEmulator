#ifndef TRADE_ACK_HANDLER_HEADER
#define TRADE_ACK_HANDLER_HEADER

#include "../../../include/Network/MainSession.h"
#include "../../../include/Network/MainSessionManager.h"
#include "Enums/GameEnums.h"
#include "../../../include/MainEnums.h"
#include "../../../include/Structures/TradeSystem/TradeAck.h"
#include "Network/Packet.h"
#include <vector>
#include <cstdint>

namespace Main
{
	namespace Handlers
	{
		inline void handleTradeAck(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager)
		{
			std::uint32_t targetAccountId;
			std::memcpy(&targetAccountId, request.getData() + sizeof(std::uint32_t), sizeof(std::uint32_t));

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());

			const auto& selfAccountInfo = session.getAccountInfo();
			Main::Structures::TradeAck tradeAck{ selfAccountInfo.uniqueId, selfAccountInfo.accountID };
			response.setData(reinterpret_cast<std::uint8_t*>(&tradeAck), sizeof(tradeAck));

			if (auto* targetSession = sessionsManager.getSessionByAccountId(targetAccountId))
			{
				auto targetPlayerState = targetSession->getPlayerState();
				const auto& targetAccountInfo = targetSession->getAccountInfo();
				if (targetAccountInfo.playerLevel < 16 || selfAccountInfo.playerLevel < 16)
				{
					response.setOrder(192);
					response.setExtra(Enums::TradeSystemExtra::LEVEL_TOO_LOW);
				}
				else if (targetPlayerState == Common::Enums::PlayerState::STATE_LOBBY || targetPlayerState == Common::Enums::PlayerState::STATE_NORMAL)
				{
					// Todo: check whether this branch is entered in case the player is in a room in a non-ready state
					targetSession->asyncWrite(response);
					return;
				}
				else
				{
					response.setOrder(192);
					response.setExtra(Enums::TradeSystemExtra::CANNOT_TRADE_NOW_OR_PLAYER_OFFLINE);
				}
				session.asyncWrite(response);
			}
		}
	}
}

#endif