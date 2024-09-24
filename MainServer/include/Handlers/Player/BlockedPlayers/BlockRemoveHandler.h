#ifndef BLOCK_REMOVAL_HANDLER_H
#define BLOCK_REMOVAL_HANDLER_H

#include "../../../Network/MainSession.h"
#include "../../../Network/MainSessionManager.h"
#include "Network/Packet.h"

namespace Main
{
	namespace Handlers
	{
		inline void handlePlayerUnblock(const Common::Network::Packet& request, Main::Network::Session& session)
		{
			std::uint32_t targetAccountId;
			std::memcpy(&targetAccountId, request.getData(), sizeof(std::uint32_t));

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setCommand(request.getOrder(), 0, session.unblockAccount(targetAccountId), 0);
			session.asyncWrite(response);
		}
	}
}

#endif