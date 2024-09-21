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
			const auto selectedCharacter = request.getOption();
			constexpr std::uint32_t characterAvailable = 1;
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());
			response.setExtra(characterAvailable); 
			response.setOption(selectedCharacter); 
			session.asyncWrite(response);
			session.setAccountLatestCharacterSelected(selectedCharacter);

			Details::broadcastPlayerItems(roomsManager, session, request);
		}
	}
}

#endif