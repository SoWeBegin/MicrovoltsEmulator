#ifndef PLAYER_STATE_HANDLER_H
#define PLAYER_STATE_HANDLER_H

#include "../../Network/MainSession.h"
#include "../../Network/MainSessionManager.h"
#include "Network/Packet.h"
#include <ConstantDatabase/Structures/SetItemInfo.h>
#include <Utils/Utils.h>

#include "../../Utilities.h"

namespace Main
{
	namespace Handlers
	{
		inline void handlePlayerState(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager)
		{
			session.setPlayerState(static_cast<Common::Enums::PlayerState>(request.getOption()));

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);

			auto foundRoom = roomsManager.getRoomByNumber(session.getRoomNumber());
			if (foundRoom == std::nullopt) return;
			auto& room = foundRoom->get();

			response.setOrder(Details::Orders::PLAYER_STATE_NOTIFICATION); 
			response.setOption(session.getPlayerState());
			auto uniqueId = session.getAccountInfo().uniqueId;
			response.setData(reinterpret_cast<std::uint8_t*>(&uniqueId), sizeof(uniqueId));
			room.broadcastToRoom(response);
			room.setStateFor(uniqueId, session.getPlayerState());

			Details::broadcastPlayerItems(roomsManager, session, request);
		}
	}
}

#endif
