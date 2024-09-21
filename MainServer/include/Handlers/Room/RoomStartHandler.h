#ifndef ROOM_START_HANDLER_HEADER
#define ROOM_START_HANDLER_HEADER

#include "../../Network/MainSession.h"
#include "../../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "Network/Packet.h"
#include "../../Classes/RoomsManager.h"

#include <chrono> 
#include "Utils/Logger.h"
#include <Utils/IPC_Structs.h>

namespace Main
{
	namespace Handlers
	{
		enum RoomStartExtra
		{
			START_SUCCESS = 38,
		};

		std::uint64_t getUtcTimeMs()
		{
			const auto durationSinceEpoch = std::chrono::system_clock::now().time_since_epoch();
			return static_cast<std::uint64_t>(duration_cast<std::chrono::milliseconds>(durationSinceEpoch).count());
		}

		inline void handleRoomStart(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager,
			std::uint64_t timeSinceLastServerRestart)
		{
			Utils::Logger& logger = Utils::Logger::getInstance();

			const auto foundRoom = roomsManager.getRoomByNumber(session.getRoomNumber());
			if (foundRoom == std::nullopt) return;
			auto& room = foundRoom.value().get(); 

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			auto selfUniqueId = session.getAccountInfo().uniqueId;

			// Click on "Start" button (match) -- host clicks start or we (as non-host) click start
			if (request.getExtra() == 38) // SingleWave: extra is 6!
			{
				session.setIsInMatch(true);
				room.startMatch(selfUniqueId);

				response.setOrder(request.getOrder());
				response.setExtra(38);
				response.setOption(room.getRoomSettings().map);
				response.setData(reinterpret_cast<std::uint8_t*>(&selfUniqueId), sizeof(selfUniqueId));
				room.broadcastToRoom(response); 

				if (room.isHost(selfUniqueId))
				{
					// Notify cast (IPC) about room's map
					Utils::MapInfo mapInfo{ room.getRoomSettings().map, selfUniqueId.session };
					Utils::IPCManager::ipc_mainToCast(mapInfo, std::to_string(room.getRoomNumber()), "map_info");

					logger.log("The player " + session.getPlayerInfoAsString() + " is also host. Main=>Cast notification about room map and mode info sent. "
						+ room.getRoomInfoAsString(), Utils::LogType::Normal, "Room::handleRoomStart");

				}
			}
			// When joining someone's match, this is sent only for specific modes (like ELI), but not for others (e.g. TDM), for which infinite match load happens???
			else if (request.getExtra() == 41)
			{
				if (room.isHost(selfUniqueId))
				{
					response.setOrder(258);
					response.setExtra(1); 
					response.setOption(0);
					struct Response
					{
						std::uint64_t tick{};
					};
					Response respMessage;
					respMessage.tick = getUtcTimeMs() - timeSinceLastServerRestart;

					response.setData(reinterpret_cast<std::uint8_t*>(&respMessage), sizeof(respMessage));
					room.setTick(respMessage.tick);
					room.broadcastToRoom(response);
					logger.log("The player " + session.getPlayerInfoAsString() + " is also host. RoomTick set and broadcast to room. "
						+ room.getRoomInfoAsString(), Utils::LogType::Normal, "Room::handleRoomStart");
				}
				else
				{
					// Tell the other players in the match that we joined
					response.setOrder(415); 
					response.setExtra(1);
					response.setData(reinterpret_cast<std::uint8_t*>(&selfUniqueId), sizeof(selfUniqueId));
					response.setMission(0);
					response.setOption(0);
					room.broadcastToRoom(response);

					logger.log("The player " + session.getPlayerInfoAsString() + " is NOT host. Notifying other players about their match join. "
						+ room.getRoomInfoAsString(), Utils::LogType::Normal, "Room::handleRoomStart");
				}
			}
		}
	}
}

#endif
