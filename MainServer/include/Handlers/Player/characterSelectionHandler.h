#ifndef CHARACTER_SELECTION_HANDLER_H
#define CHARACTER_SELECTION_HANDLER_H

#include "../../Network/MainSession.h"
#include "../../Classes/RoomsManager.h"
#include "Network/Packet.h"
#include "../../MainEnums.h"
#include "../../Utilities.h"

namespace Main
{
	namespace Handlers
	{
		inline void handleCharacterSelection(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager)
		{
			constexpr bool isCharacterAvailable = true;

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setCommand(request.getOrder(), 0, isCharacterAvailable, request.getOption());
			session.asyncWrite(response);

			session.setAccountLatestCharacterSelected(request.getOption());
			Details::broadcastPlayerItems(roomsManager, session, request);
		}
	}
}

#endif