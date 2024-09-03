#ifndef ROOM_JOIN_HANDLER_H
#define ROOM_JOIN_HANDLER_H

#include "../../Network/MainSession.h"
#include "Network/Packet.h"
#include "../../Classes/RoomsManager.h"
#include "../../Structures/Room/RoomJoinLatestInfo.h"
#include <Enums/PlayerEnums.h>
#include "Utils/Logger.h"

namespace Main
{
	namespace Handlers
	{
		enum RoomJoinOrder
		{
			RoomInfo = 139,
			RoomLatestEnteredPlayerInfo = 421,
			RoomPlayersInfos = 406,
			RoomPlayersItems = 303,
			RoomLatestInfo = 309,
			RoomPlayersClans = 409,
		};

		enum RoomJoinExtra
		{
			JOIN_OBSERVER_MODE = 0, // if mission 0 obs, if mission 1 apparently invisible mode or network error?
			JOIN_SUCCESS = 1,
			JOIN_ROOM_IS_DELETED = 6, // "The lobby has been deleted, you have been disconnected"
			JOIN_NO_PERMISSION = 2, // "Error"
			JOIN_BEGINNERSERVER = 5, // "Unable to enter room"
			JOIN_LOBBY_FULL = 8, // "Lobby is full, please try again later"
			JOIN_ASK_PASSWORD = 0xE, // apparently this asks the user to input a password (even if the room doesn't have it) -- but the client already does this
			JOIN_LEVEL_TOO_HIGH = 0x15, // the host of the room has a low level, higher level players cannot join their room
			JOIN_KICKED_FROM_ROOM = 0x2A,
			JOIN_INVALID_PASSWORD = 0x2C, 
		};

		// N.B GM grade can't enter observer mode at all seemingly (it's literally disabled)
		// Checked.
		inline void handleRoomJoin(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager)
		{
			Utils::Logger& logger = Utils::Logger::getInstance();

			struct RequestStructure
			{
				std::uint16_t roomNumber{};
				std::uint16_t unknown{};
				char password[8]{};
			} requestStructure;
			std::memcpy(&requestStructure, request.getData(), sizeof(requestStructure));

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());

			auto foundRoom = roomsManager.getRoomByNumber(requestStructure.roomNumber + 1);
			const auto& accountInfo = session.getAccountInfo();

			if (foundRoom == std::nullopt)
			{
				logger.log("The player " + session.getPlayerInfoAsString() + " attempted to join a room that was deleted. ",
					Utils::LogType::Warning, "Room::roomJoinHandler");

				response.setExtra(RoomJoinExtra::JOIN_ROOM_IS_DELETED);
				session.asyncWrite(response);
				return;
			}
			auto& room = foundRoom.value().get();
			auto roomInfo = room.getRoomJoinInfo();
			const auto& roomSettings = room.getRoomSettings();

			// 1. Check password - mods/tester/gm can join in rooms with passwords
			if (accountInfo.playerGrade >= Common::Enums::PlayerGrade::GRADE_MOD && roomInfo.hasPassword)
			{
				std::memcpy(roomInfo.password, room.getPassword().c_str(), 9);
			}
			else if (roomSettings.hasPassword && room.getPassword() != std::string{ requestStructure.password })
			{
				logger.log("The player " + session.getPlayerInfoAsString() + " attempted to join a room with an incorrect password. "
					+ room.getRoomInfoAsString(), Utils::LogType::Warning, "Room::roomJoinHandler");

				response.setExtra(RoomJoinExtra::JOIN_INVALID_PASSWORD);
				session.asyncWrite(response);
				return;
			}

			// 2. Check whether the room is full. If so, try to join the observer team if it isn't full too. Otherwise, one cannot enter the room
			bool joinedAsObserver = false;
			if (room.isRoomFullObserverExcluded())
			{
				if (room.isObserverFull() || !roomSettings.isObserverModeOn)
				{
					logger.log("The player " + session.getPlayerInfoAsString() + " attempted to join a room that is full. "
						+ room.getRoomInfoAsString(), Utils::LogType::Warning, "Room::roomJoinHandler");

					response.setExtra(RoomJoinExtra::JOIN_LOBBY_FULL);
					session.asyncWrite(response);
					return;
				}
				else
				{
					joinedAsObserver = true;
				}
			}

			// 3. Check whether the player was previously kicked from this room
			if (room.wasPreviouslyKicked(accountInfo.accountID))
			{
				logger.log("The player " + session.getPlayerInfoAsString() + " attempted to join a room where they were previously kicked. "
					+ room.getRoomInfoAsString(), Utils::LogType::Warning, "Room::roomJoinHandler");

				response.setExtra(RoomJoinExtra::JOIN_KICKED_FROM_ROOM);
				session.asyncWrite(response);
				return;
			}

			// 4. Check whether the host of the room is a low level (<= 15), and whether whoever joined the room is higher than that.
			// In that case, do not allow it.
			if (room.getHostLevel() <= 16)
			{
				if (session.getAccountInfo().playerLevel > 16)
				{
					response.setExtra(RoomJoinExtra::JOIN_LEVEL_TOO_HIGH);
					session.asyncWrite(response);
					return;
				}
			}

			response.setOrder(RoomJoinOrder::RoomInfo);
			// Extra + Option = uint16_t = Room Number
			response.setExtra(room.getRoomInfo().roomNumber);
			response.setOption(0); // room number (unnedeed for now, since max rooms in one server is 30 which is enough for the extra)
			response.setMission(room.getRoomInfo().hasPassword);
			response.setData(reinterpret_cast<std::uint8_t*>(&roomInfo), sizeof(roomInfo));
			session.asyncWrite(response);

			// Player infos
			response.setOrder(RoomJoinOrder::RoomPlayersInfos);
			response.setExtra(37);
			auto allPlayers = room.getAllPlayers();
			response.setOption(allPlayers.size()); // num of players inside the room (before we join)
			response.setData(reinterpret_cast<std::uint8_t*>(allPlayers.data()), sizeof(Main::Structures::RoomPlayerInfo) * allPlayers.size());
			session.asyncWrite(response);


			// Player items -- This can be > 1400 (MAX PACKET SIZE) bytes!
			response.setOrder(RoomJoinOrder::RoomPlayersItems);

			constexpr std::size_t headerSize = sizeof(Common::Protocol::TcpHeader) + sizeof(Common::Protocol::CommandHeader);
			auto allPlayersItems = room.getPlayersItems();
			std::size_t totalSize = headerSize + sizeof(Main::Structures::RoomPlayerItems) * allPlayersItems.size();
			constexpr std::size_t MAX_PACKET_SIZE = 1440;

			if (totalSize < MAX_PACKET_SIZE)
			{
				response.setOption(allPlayersItems.size());
				response.setExtra(37);
				response.setData(reinterpret_cast<std::uint8_t*>(allPlayersItems.data()), sizeof(Main::Structures::RoomPlayerItems) * allPlayersItems.size());
				session.asyncWrite(response);
			}
			else
			{
				std::size_t currentItemIndex = 0;
				std::uint16_t packetExtra = 0;
				const std::size_t maxPayloadSize = MAX_PACKET_SIZE - headerSize;
				const std::size_t itemsToSend = maxPayloadSize / sizeof(Main::Structures::RoomPlayerItems);

				while (currentItemIndex < allPlayersItems.size())
				{
					std::vector<Main::Structures::RoomPlayerItems> packetItems(
						allPlayersItems.begin() + currentItemIndex,
						allPlayersItems.begin() + std::min(currentItemIndex + itemsToSend, allPlayersItems.size())
					);

					packetExtra = currentItemIndex == 0 ? 37 : 0;
					response.setExtra(packetExtra);
					response.setData(reinterpret_cast<std::uint8_t*>(packetItems.data()), packetItems.size() * sizeof(Main::Structures::RoomPlayerItems));
					response.setOption(packetItems.size());
					session.asyncWrite(response);

					currentItemIndex += itemsToSend;
				}
			}

			// Player clan infos
			// option => again num of players (= num of clans)
			response.setOrder(RoomJoinOrder::RoomPlayersClans);
			response.setExtra(37);
			auto allPlayersClans = room.getPlayersClans();
			response.setOption(allPlayersClans.size());
			response.setMission(0); // TODO: check what is mission(1) in IDA
			response.setData(reinterpret_cast<std::uint8_t*>(allPlayersClans.data()), sizeof(Main::Structures::PlayerClan) * allPlayersClans.size());
			session.asyncWrite(response);

			// Success in joining the room
			response.setOrder(request.getOrder());
			response.setData(nullptr, 0);
			response.setOption(1);
			response.setMission(0);
			// extra 0, mission 0 ==> observer mode
			// extra 0, mission 1 ==> seemingly invisible mode?!
			response.setExtra(joinedAsObserver ? RoomJoinExtra::JOIN_OBSERVER_MODE : RoomJoinExtra::JOIN_SUCCESS);
			session.asyncWrite(response);

			// THIS SEEMS WRONG. FIRST IT NEEDS TO BE SENT BEFORE "SUCCESS IN JOINING THE ROOM" PROBABLY; BUT THEN SETTINGS ARE WRONG.
			/*
			// Latest, missing information about the room
			Main::Structures::RoomJoinLatestInfo roomJoinLatestInfo{ room.getSpecificSetting(), static_cast<bool>(roomInfo.hasMatchStarted), static_cast<bool>(roomSettings.isItemOn),
				static_cast<std::uint16_t>(roomSettings.time), roomSettings.weaponRestriction };
			response.setOrder(RoomJoinOrder::RoomLatestInfo);
			response.setOption(roomSettings.mode); // mode
			response.setMission(0);
			response.setExtra(0);
			response.setData(reinterpret_cast<std::uint8_t*>(&roomJoinLatestInfo), sizeof(roomJoinLatestInfo));
			session.asyncWrite(response);
			*/

			// Send the info of the player that just joined to the whole room
			Main::Structures::RoomLatestEnteredPlayerInfo latestEnteredPlayerInfo;
			latestEnteredPlayerInfo.character = accountInfo.latestSelectedCharacter;
			latestEnteredPlayerInfo.level = accountInfo.playerLevel;
			latestEnteredPlayerInfo.ping = session.getPing();
			latestEnteredPlayerInfo.uniqueId = accountInfo.uniqueId;
			std::memcpy(latestEnteredPlayerInfo.playerName, accountInfo.nickname, 16);
			auto separatedItems = session.getEquippedItemsSeparated(); // first=items, second=weapons
			latestEnteredPlayerInfo.equippedItems = separatedItems.first;
			latestEnteredPlayerInfo.equippedWeapons = separatedItems.second;
			if (joinedAsObserver)
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
			response.setOrder(RoomJoinOrder::RoomLatestEnteredPlayerInfo);
			response.setExtra(0);
			response.setMission(0);
			response.setOption(1);
			response.setData(reinterpret_cast<std::uint8_t*>(&latestEnteredPlayerInfo), sizeof(latestEnteredPlayerInfo));
			room.broadcastToRoom(response);

			if (joinedAsObserver)
			{
				logger.log("The player " + session.getPlayerInfoAsString() + " joined the room as an observer player. "
					+ room.getRoomInfoAsString(), Utils::LogType::Normal, "Room::roomJoinHandler");

				room.addObserverPlayer(&session);
			}
			else
			{
				logger.log("The player " + session.getPlayerInfoAsString() + " joined the room normally. "
					+ room.getRoomInfoAsString(), Utils::LogType::Normal, "Room::roomJoinHandler");

				room.addPlayer(&session, latestEnteredPlayerInfo.team);
			}

			logger.log("The player " + session.getPlayerInfoAsString() + " has joined a room. "
				+ room.getRoomInfoAsString(), Utils::LogType::Normal, "Room::roomJoinHandler");
		}
	}
}

#endif