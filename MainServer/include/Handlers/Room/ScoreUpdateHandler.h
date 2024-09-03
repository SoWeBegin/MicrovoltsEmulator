#ifndef ROOM_SCORE_UPDATE_HANDLER
#define ROOM_SCORE_UPDATE_HANDLER

#include "../../Network/MainSession.h"
#include "Network/Packet.h"
#include "../../Classes/RoomsManager.h"
#include <array>

namespace Main
{
	namespace Handlers
	{
		// battery of the user, not CTB
		inline void handleBattery(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager)
		{
			auto room = roomsManager.getRoomByNumber(session.getRoomNumber());
			if (room == std::nullopt) return;
			auto& actualRoom = room->get();

			auto esponse = request;
			std::srand(static_cast<unsigned>(std::time(nullptr)));
			std::uint32_t random = std::rand() % 4;
			if (random == 0) return; // No battery needs to be given

			Main::Structures::UniqueId targetUniqueId;
			auto data = request.getData();
			std::memcpy(&targetUniqueId, data, sizeof(targetUniqueId));

			auto* targetSession = actualRoom.getPlayer(targetUniqueId);
			if (targetSession)
			{
				const std::array<std::uint32_t, 4> batteries{ 0, 30, 50, 100 };
				Common::Network::Packet response;
				response.setTcpHeader(targetSession->getId(), Common::Enums::USER_LARGE_ENCRYPTION);
				response.setOrder(request.getOrder());
				response.setExtra(1);
				response.setOption(batteries[random]);
				targetSession->asyncWrite(response);
				targetSession->addBatteryObtainedInMatch(batteries[random]);
			}
		}
	}
}

#endif