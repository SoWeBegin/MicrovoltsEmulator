#ifndef PING_HANDLER_H
#define PING_HANDLER_H

#include "../../Network/MainSession.h"
#include "../../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "Network/Packet.h"
#include "../../../include/Classes/RoomsManager.h"
#include "../../Structures/ClientData/Structures.h"

namespace Main
{
	namespace Handlers
	{
		inline void handlePing(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager,
			const Main::ClientData::Ping& pingData)
		{
			if (request.getMission() == 1)
			{
				if (Main::Classes::Room* room = roomsManager.getRoomByNumber(session.getRoomNumber()))
				{
					session.setPing(pingData.ping);
					struct Resp
					{
						Main::ClientData::Ping clientData;
						Main::Structures::UniqueId uniqueId{};
					};
					Resp resp{ pingData, session.getAccountInfo().uniqueId };

					Common::Network::Packet response;
					response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
					response.setCommand(request.getOrder() + 1, request.getMission() + 1, 0, request.getOption());
					response.setData(reinterpret_cast<std::uint8_t*>(&resp), sizeof(resp));
					room->broadcastToRoomExceptSelf(response, resp.uniqueId);
				}
			}
		}
	}
}

#endif
