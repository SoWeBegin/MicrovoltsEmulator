#ifndef CHARACTER_SELECTION_HANDLER_H
#define CHARACTER_SELECTION_HANDLER_H

#include "../../Network/MainSession.h"
#include "../../Classes/RoomsManager.h"
#include "Network/Packet.h"
#include "../../MainEnums.h"
#include <Utils/Utils.h>

namespace Main
{
	namespace Handlers
	{
		inline void handleCharacterSelection(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager)
		{
			session.setAccountLatestCharacterSelected(request.getOption());
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());
			response.setExtra(1); // 1 == character available (elaborate this)
			response.setOption(request.getOption()); // selected character
			session.asyncWrite(response);
		}
	}
}

#endif