#ifndef TRADE_CANCEL_HANDLER_H
#define TRADE_CANCEL_HANDLER_H

#include "../../../include/Network/MainSession.h"
#include "../../../include/Network/MainSessionManager.h"
#include "../../../include/MainEnums.h"
#include "../../../include/Structures/TradeSystem/TradeAck.h"
#include "Network/Packet.h"
#include <cstdint>

namespace Main
{
	namespace Handlers
	{
		inline void handleTradeCancellation(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager)
		{
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());

			Main::Structures::TradeAck ack{ Main::Structures::UniqueId{}, session.getAccountInfo().accountID };
			
			if (auto* targetSession = sessionsManager.getSessionByAccountId(session.getCurrentlyTradingWithAccountId()))
			{
				response.setExtra(Enums::TradeSystemExtra::TRADE_CANCELLED); // This player exited the trade
				targetSession->asyncWrite(response);
				session.setPlayerState(Common::Enums::PlayerState::STATE_INVENTORY);
				targetSession->setPlayerState(Common::Enums::PlayerState::STATE_INVENTORY);
				session.resetTradeInfo();
				targetSession->resetTradeInfo();
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