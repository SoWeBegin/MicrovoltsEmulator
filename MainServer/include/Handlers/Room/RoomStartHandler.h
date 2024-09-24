#ifndef ROOM_START_HANDLER_HEADER
#define ROOM_START_HANDLER_HEADER

#include "../../Network/MainSession.h"
#include "../../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "Network/Packet.h"
#include "../../Classes/RoomsManager.h"
#include <Utils/IPC_Structs.h>

namespace Main
{
	namespace Handlers
	{
		std::uint64_t getUtcTimeMs()
		{
			const auto durationSinceEpoch = std::chrono::system_clock::now().time_since_epoch();
			return static_cast<std::uint64_t>(duration_cast<std::chrono::milliseconds>(durationSinceEpoch).count());
		}

		inline void handleRoomStart(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager,
			std::uint64_t timeSinceLastServerRestart)
		{
			const auto foundRoom = roomsManager.getRoomByNumber(session.getRoomNumber());
			if (foundRoom == std::nullopt) return;
			auto& room = foundRoom.value().get(); 

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			auto selfUniqueId = session.getAccountInfo().uniqueId;

			if (request.getExtra() == 38) // host or non-host clicks on "start" button (n.b: SingleWave's extra is 6)
			{
				response.setCommand(request.getOrder(), 0, 38, room.getRoomSettings().map);
				response.setData(reinterpret_cast<std::uint8_t*>(&selfUniqueId), sizeof(selfUniqueId));
				room.broadcastToRoom(response); 

				if (room.isHost(selfUniqueId))
				{
					Utils::MapInfo mapInfo{ room.getRoomSettings().map, selfUniqueId.session };
					Utils::IPCManager::ipc_mainToCast(mapInfo, std::to_string(room.getRoomNumber()), "map_info");
				}

				// 421
				auto accountInfo = session.getAccountInfo();
				Main::Structures::RoomLatestEnteredPlayerInfo latestEnteredPlayerInfo;
				latestEnteredPlayerInfo.character = accountInfo.latestSelectedCharacter;
				latestEnteredPlayerInfo.level = accountInfo.playerLevel;
				latestEnteredPlayerInfo.ping = session.getPing();
				latestEnteredPlayerInfo.uniqueId = accountInfo.uniqueId;
				std::memcpy(latestEnteredPlayerInfo.playerName, accountInfo.nickname, 16);
				auto separatedItems = session.getEquippedItemsSeparated(); // first=items, second=weapons
				latestEnteredPlayerInfo.equippedItems = separatedItems.first;
				latestEnteredPlayerInfo.equippedWeapons = separatedItems.second;
				if (false) // joined as observer
				{
					latestEnteredPlayerInfo.team = Common::Enums::TEAM_OBSERVER;
				}
				else
				{
					if (room.isModeTeamBased())
					{
						latestEnteredPlayerInfo.team = room.calculateNewPlayerTeam();
					}
					else
					{
						latestEnteredPlayerInfo.team = Common::Enums::Team::TEAM_ALL;
					}
				}
				response.setCommand(RoomJoinOrder::RoomLatestEnteredPlayerInfo, 0, 0, 0);
				response.setData(reinterpret_cast<std::uint8_t*>(&latestEnteredPlayerInfo), sizeof(latestEnteredPlayerInfo));
				room.broadcastToRoom(response);

				// 409
				auto roomSettings = room.getRoomSettings();
				response.setCommand(RoomJoinOrder::RoomLatestInfo, 0, 0, roomSettings.mode);
				if (roomSettings.mode == Common::Enums::FreeForAll)
				{
					Main::Structures::ModeInfoFFA info;
					info.state = room.hasMatchStarted() + 1;
					info.timelimited = roomSettings.time;
					info.weaponlimited = roomSettings.weaponRestriction;
					info.winrule = room.getSpecificSetting();
					info.team_balance = false;
					response.setData(reinterpret_cast<std::uint8_t*>(&info), sizeof(info));
				}
				else if (roomSettings.mode == Common::Enums::Scrimmage)
				{
					Main::Structures::ModeInfoScrimmage info;
					info.state = room.hasMatchStarted() + 1;
					info.timelimited = roomSettings.time;
					info.weaponlimited = roomSettings.weaponRestriction;
					response.setData(reinterpret_cast<std::uint8_t*>(&info), sizeof(info));
				}
				else
				{
					Main::Structures::ModeInfoTDM info;
					info.state = room.hasMatchStarted() + 1;
					info.timelimited = roomSettings.time;
					info.weaponlimited = roomSettings.weaponRestriction;
					info.winrule = room.getSpecificSetting();
					response.setData(reinterpret_cast<std::uint8_t*>(&info), sizeof(info));
				}
				session.asyncWrite(response);

				// 312
				response.setOrder(312);
				response.setOption(11);
				response.setMission(0);
				response.setExtra(0);
				response.setData(reinterpret_cast<std::uint8_t*>(&accountInfo.uniqueId), sizeof(accountInfo));
				session.asyncWrite(response);
			}
			else if (request.getExtra() == 41)
			{
				session.setIsInMatch(true);

				if (room.isHost(selfUniqueId)) // broadcast the tick to the room
				{
					std::uint64_t roomTick = getUtcTimeMs() - timeSinceLastServerRestart;
					room.setTick(roomTick);
					response.setCommand(258, 0, 1, 0);
					response.setData(reinterpret_cast<std::uint8_t*>(&roomTick), sizeof(roomTick));
					room.broadcastToRoom(response);
					room.startMatch(selfUniqueId);
				}
				else // Tell the other players in the match that we joined
				{
					response.setCommand(415, 0, 1, 0);
					response.setData(reinterpret_cast<std::uint8_t*>(&selfUniqueId), sizeof(selfUniqueId));
					room.broadcastToRoom(response);
				}
			}
		}
	}
}

#endif
