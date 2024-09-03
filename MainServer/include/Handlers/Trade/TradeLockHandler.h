#ifndef TRADE_LOCK_HANDLER_HEADER
#define TRADE_LOCK_HANDLER_HEADER

#include "../../../include/Network/MainSession.h"
#include "../../../include/Network/MainSessionManager.h"
#include "../../../include/MainEnums.h"
#include "Enums/GameEnums.h"
#include "Network/Packet.h"
#include <vector>
#include <cstdint>

namespace Main
{
	namespace Handlers
	{
		inline void handleTradeLock(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager)
		{
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());
			response.setExtra(Enums::TradeSystemExtra::TRADE_SUCCESS);

			if (auto* targetSession = sessionsManager.getSessionByAccountId(session.getCurrentlyTradingWithAccountId()))
			{
				targetSession->asyncWrite(response);
			}
			else
			{
				response.setExtra(Enums::TradeSystemExtra::CANNOT_TRADE_NOW_OR_PLAYER_OFFLINE);
				session.asyncWrite(response);
				session.resetTradeInfo();
			}
		}
	}
}

#endif