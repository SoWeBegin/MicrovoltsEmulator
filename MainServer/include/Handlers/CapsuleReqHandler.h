#ifndef CAPSULE_REQ_HANDLER_H
#define CAPSULE_REQ_HANDLER_H

#include "../Network/MainSession.h"
#include "../Network/MainSessionManager.h"
#include "Network/Packet.h"
#include <ConstantDatabase/Structures/SetItemInfo.h>
#include <Utils/Utils.h>

#include "../Utilities.h"

namespace Main
{
	namespace Handlers
	{
		inline void handleCapsuleReq(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager)
		{
			// capsules are all completely client sided
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(83);
			std::uint32_t mpJackpot = 0;
			response.setData(reinterpret_cast<std::uint8_t*>(&mpJackpot), sizeof(mpJackpot));
			session.asyncWrite(response);
		}
	}
}

#endif

