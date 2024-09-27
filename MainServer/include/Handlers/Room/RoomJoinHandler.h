#ifndef ROOM_JOIN_HANDLER_H
#define ROOM_JOIN_HANDLER_H

#include "../../Network/MainSession.h"
#include "Network/Packet.h"
#include "../../Classes/RoomsManager.h"
#include "../../Structures/Room/RoomJoinLatestInfo.h"
#include <Enums/PlayerEnums.h>
#include "../../Structures/Room/RoomJoinLatestInfo.h"

namespace Main
{
	namespace Handlers
	{
		enum RoomJoinOrder
		{
			RoomInfo = 135, // ok
			RoomLatestEnteredPlayerInfo = 421, // this
			RoomPlayersInfos = 406,
			RoomPlayersItems = 303,
			RoomLatestInfo = 309,  // this
			RoomPlayersClans = 409, // this
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
		inline void handleRoomJoin(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager,
			const Main::ClientData::RoomInfo& requestStructure, bool roomJoinCheckPassword = false)
		{
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());

			if (Main::Classes::Room* room = roomsManager.getRoomByNumber(requestStructure.roomNumber + 1))
			{
				auto roomInfo = room->getRoomJoinInfo();
				const auto& roomSettings = room->getRoomSettings();

				if (roomJoinCheckPassword)
				{
					response.setExtra(RoomJoinExtra::JOIN_ASK_PASSWORD);
					session.asyncWrite(response);
					return;
				}

				const auto& accountInfo = session.getAccountInfo();
				// 1. Check password - mods/tester/gm can join in rooms with passwords
				if (accountInfo.playerGrade >= Common::Enums::PlayerGrade::GRADE_MOD && roomInfo.hasPassword)
				{
					std::memcpy(roomInfo.password, room->getPassword().c_str(), 9);
				}
				else if (roomSettings.hasPassword && room->getPassword() != std::string{ requestStructure.password })
				{
					response.setExtra(RoomJoinExtra::JOIN_INVALID_PASSWORD);
					session.asyncWrite(response);
					return;
				}

				// 2. Check whether the room is full. If so, try to join the observer team if it isn't full too. Otherwise, one cannot enter the room
				bool joinedAsObserver = false;
				if (room->isRoomFullObserverExcluded())
				{
					if (room->isObserverFull() || !roomSettings.isObserverModeOn)
					{
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
				if (room->wasPreviouslyKicked(accountInfo.accountID))
				{
					response.setExtra(RoomJoinExtra::JOIN_KICKED_FROM_ROOM);
					session.asyncWrite(response);
					return;
				}

				// 4. Check whether the host of the room is a low level (<= 15), and whether whoever joined the room is higher than that.
				// In that case, do not allow it.
				if (room->getHostLevel() <= 16)
				{
					if (session.getAccountInfo().playerLevel > 16)
					{
						response.setExtra(RoomJoinExtra::JOIN_LEVEL_TOO_HIGH);
						session.asyncWrite(response);
						return;
					}
				}

				response.setCommand(RoomJoinOrder::RoomInfo, room->getRoomInfo().hasPassword, room->getRoomInfo().roomNumber, 8); // Extra + Option = uint16_t = Room Number
				response.setData(reinterpret_cast<std::uint8_t*>(&roomInfo), sizeof(roomInfo));
				session.asyncWrite(response);

				// Player infos
				auto allPlayers = room->getAllPlayers();
				response.setCommand(RoomJoinOrder::RoomPlayersInfos, room->getRoomInfo().hasPassword, 37, allPlayers.size());
				response.setData(reinterpret_cast<std::uint8_t*>(allPlayers.data()), sizeof(Main::Structures::RoomPlayerInfo) * allPlayers.size());
				session.asyncWrite(response);

				// Player items -- This can be > 1400 (MAX PACKET SIZE) bytes!
				response.setOrder(RoomJoinOrder::RoomPlayersItems);

				constexpr std::size_t headerSize = sizeof(Common::Protocol::TcpHeader) + sizeof(Common::Protocol::CommandHeader);
				auto allPlayersItems = room->getPlayersItems();
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
						response.setOption(packetItems.size());
						response.setData(reinterpret_cast<std::uint8_t*>(packetItems.data()), packetItems.size() * sizeof(Main::Structures::RoomPlayerItems));
						session.asyncWrite(response);

						currentItemIndex += itemsToSend;
					}
				}

				// Player clan infos
				// option => again num of players (= num of clans)
				auto allPlayersClans = room->getPlayersClans();
				response.setCommand(RoomJoinOrder::RoomPlayersClans, 0, 37, allPlayersClans.size());
				response.setData(reinterpret_cast<std::uint8_t*>(allPlayersClans.data()), sizeof(Main::Structures::PlayerClan) * allPlayersClans.size());
				session.asyncWrite(response);

				// Success in joining the room
				response.setCommand(request.getOrder(), 0, joinedAsObserver ? RoomJoinExtra::JOIN_OBSERVER_MODE : RoomJoinExtra::JOIN_SUCCESS, 0);
				response.setData(nullptr, 0);
				// extra 0, mission 0 ==> observer mode
				// extra 0, mission 1 ==> seemingly invisible mode?!
				session.asyncWrite(response);

				// Latest, missing information about the room
				response.setCommand(RoomJoinOrder::RoomLatestInfo, 0, 0, roomSettings.mode);
				if (roomSettings.mode == Common::Enums::FreeForAll)
				{
					Main::Structures::ModeInfoFFA info;
					info.state = room->hasMatchStarted() ? 3 : 0;
					info.timelimited = roomSettings.time;
					info.weaponlimited = roomSettings.weaponRestriction;
					info.winrule = room->getSpecificSetting();
					info.team_balance = false;
					response.setData(reinterpret_cast<std::uint8_t*>(&info), sizeof(info));
				}
				else if (roomSettings.mode == Common::Enums::Scrimmage)
				{
					Main::Structures::ModeInfoScrimmage info;
					info.state = room->hasMatchStarted() ? 3 : 0;
					info.timelimited = roomSettings.time;
					info.weaponlimited = roomSettings.weaponRestriction;
					response.setData(reinterpret_cast<std::uint8_t*>(&info), sizeof(info));
				}
				else
				{
					Main::Structures::ModeInfoTDM info;
					info.state = room->hasMatchStarted() ? 3 : 0;
					info.timelimited = roomSettings.time;
					info.weaponlimited = roomSettings.weaponRestriction;
					info.winrule = room->getSpecificSetting();
					info.kitdrop = 1;
					response.setData(reinterpret_cast<std::uint8_t*>(&info), sizeof(info));
				}
				session.asyncWrite(response);

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
					if (room->isModeTeamBased())
					{
						latestEnteredPlayerInfo.team = room->calculateNewPlayerTeam();
					}
					else
					{
						latestEnteredPlayerInfo.team = Common::Enums::Team::TEAM_ALL;
					}
				}
				response.setCommand(RoomJoinOrder::RoomLatestEnteredPlayerInfo, 0, 0, 1);
				response.setData(reinterpret_cast<std::uint8_t*>(&latestEnteredPlayerInfo), sizeof(latestEnteredPlayerInfo));
				room->broadcastToRoom(response);

				if (joinedAsObserver)
				{
					room->addObserverPlayer(&session);
				}
				else
				{
					room->addPlayer(&session, latestEnteredPlayerInfo.team);
				}
			}
			else
			{
				response.setExtra(RoomJoinExtra::JOIN_ROOM_IS_DELETED);
				session.asyncWrite(response);
			}
		}
	}
}

#endif