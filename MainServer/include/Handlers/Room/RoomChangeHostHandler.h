#ifndef ROOM_CHANGE_HOST_HANDLER
#define ROOM_CHANGE_HOST_HANDLER

#include "../../Network/MainSession.h"
#include "Network/Packet.h"
#include "../../Classes/RoomsManager.h"
#include "Utils/Logger.h"

namespace Main
{
	namespace Handlers
	{
		enum RoomChangeHostExtra
		{
			CHANGE_HOST_SUCCESS = 1,
			CHANGE_HOST_FAIL = 2,
			CHANGE_HOST_DOESNT_EXIST = 0xD,
			CHANGE_HOST_NO_PERMISSION = 0x10,
		};

		// Checked.
		inline void handleHostChange(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager)
		{
			Utils::Logger& logger = Utils::Logger::getInstance();

			auto foundRoom = roomsManager.getRoomByNumber(session.getRoomNumber());
			if (foundRoom == std::nullopt) return;
			auto& room = foundRoom.value().get();

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());
			response.setOption(request.getOption());
			response.setExtra(RoomChangeHostExtra::CHANGE_HOST_SUCCESS);

			logger.log("Attempting to change the host to m_players idx: " + std::to_string((uint32_t)request.getOption()) + room.getRoomInfoAsString(),
				Utils::LogType::Normal, "Main::handleHostChange");

			const bool changed = room.changeHost(request.getOption());
			if (!changed)
			{
				logger.log("Attempt to change the host failed.", Utils::LogType::Warning, "Main::handleHostChange");

				response.setExtra(RoomChangeHostExtra::CHANGE_HOST_DOESNT_EXIST);
				session.asyncWrite(response);
			}
			else
			{
				room.broadcastToRoom(const_cast<Common::Network::Packet&>(request));
			}
		}
	}
}

#endif