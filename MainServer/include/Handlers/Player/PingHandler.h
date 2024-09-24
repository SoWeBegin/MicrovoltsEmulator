#ifndef PING_HANDLER_H
#define PING_HANDLER_H

#include "../../Network/MainSession.h"
#include "../../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "Network/Packet.h"
#include "../../../include/Classes/RoomsManager.h"
#include "../../Structures/ClientData/PingData.h"

namespace Main
{
	namespace Handlers
	{
		inline void handlePing(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager,
			const Main::ClientData::Ping& pingData)
		{
			if (request.getMission() == 1)
			{
				session.setPing(pingData.ping);
				const std::uint16_t roomNumber = session.getRoomNumber();

				if (roomNumber)
				{
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

					auto roomFound = roomsManager.getRoomByNumber(roomNumber);
					if (roomFound == std::nullopt) return;
					auto& room = roomFound->get();
					room.broadcastToRoomExceptSelf(response, resp.uniqueId);
				}
			}
		}
	}
}

#endif
