#ifndef PING_HANDLER_H
#define PING_HANDLER_H

#include "../../Network/MainSession.h"
#include "../../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "Network/Packet.h"
#include "../../../include/Classes/RoomsManager.h"

namespace Main
{
	namespace Handlers
	{
		inline void handlePing(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager)
		{
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder() + 1);
			response.setMission(request.getMission());
			response.setOption(request.getOption());
			// maybe set extra?

			if (request.getMission() == 1)
			{
				struct Req
				{
					std::uint32_t unused : 10 = 0; // recheck whether this really is unused, apparently it IS used
					std::uint32_t ping : 10 = 0;
					std::uint32_t rest : 12 = 0;
				} clientReq;
				std::memcpy(&clientReq, request.getData(), request.getDataSize());
				session.setPing(clientReq.ping);

				const std::uint16_t roomNumber = session.getRoomNumber();
				if (roomNumber)
				{
					struct Resp
					{
						Req clientReq;
						Main::Structures::UniqueId uniqueId{};
					} resp;
					resp.clientReq = clientReq;
					resp.uniqueId = session.getAccountInfo().uniqueId;
					response.setMission(2);
					response.setData(reinterpret_cast<std::uint8_t*>(&resp), sizeof(resp));

					auto roomFound = roomsManager.getRoomByNumber(roomNumber);
					if (roomFound == std::nullopt) return;
					auto& room = roomFound.value().get();
					room.broadcastToRoomExceptSelf(response, resp.uniqueId);
				}
			}
		}
	}
}

#endif