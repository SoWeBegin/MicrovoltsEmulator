#ifndef ROOM_MISC_HANDLER_HEADER
#define ROOM_MISC_HANDLER_HEADER

#include "../../Network/MainSession.h"
#include "../../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "../../../include/Structures/Room/RoomSettingsUpdate.h"
#include "Network/Packet.h"
#include <span>
#include "../Room/RoomJoinHandler.h"
#include "../Room/RoomLeaveHandler.h"


namespace Main
{
	namespace Handlers
	{
		enum RoomInviteJoinExtra
		{
			USER_INVITING_FRIEND = 0,
			USER_READY = 5,
			USER_OFFLINE = 0xD,
			ROOM_FULL = 0xE,
			ROOM_SENDINVITE_TOTARGET = 44,
			USER_JOINING_FRIEND = 28,
		};

		inline void unknown(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager, 
			Main::Network::SessionsManager& sessionsManager)
		{
			auto response = request;

			if (request.getExtra() == RoomInviteJoinExtra::USER_JOINING_FRIEND) 
			{
				response.setOrder(136);
				std::uint32_t targetAccountId;
				std::memcpy(&targetAccountId, reinterpret_cast<std::uint8_t*>(const_cast<std::uint8_t*>(request.getData())), sizeof(targetAccountId));
				auto* targetSession = sessionsManager.getSessionByAccountId(targetAccountId);
				if (targetSession)
				{
					const std::uint32_t targetRoomNum = targetSession->getRoomNumber();
					const std::uint32_t selfRoomNum = session.getRoomNumber();
					if (targetRoomNum)
					{
						if (!session.isInLobby() && !selfRoomNum)
						{ // User is neither in lobby and in room, join should not work in this case
							Main::Details::sendMessage("You can only join a friend's room while being in a different room or in the lobby!", session);
						    return;
						}
						else if (targetRoomNum == selfRoomNum)
						{ // User attempting to join the same room, disallow this
							Main::Details::sendMessage("You cannot join this friend as you're already in their room.", session);
							return;
						} 
						else if (selfRoomNum)
						{ // if the player is in a room, leave it first
							Common::Network::Packet response;
							response.setTcpHeader(session.getId(), Common::Enums::USER_LARGE_ENCRYPTION);
							response.setOrder(137); 
							Main::Handlers::handleRoomLeave(response, session, sessionsManager, roomsManager);
						}

						struct JoinInfo
						{
							std::uint16_t roomNum = 0;
							std::uint16_t unknown = 2; // seemingly always 2 for some reason
						};

						JoinInfo joinInfo{ targetRoomNum - 1 };
						response.setOption(0);
						response.setExtra(0);
						response.setMission(0);
						response.setData(reinterpret_cast<std::uint8_t*>(&joinInfo), sizeof(joinInfo));

						auto roomOpt = roomsManager.getRoomByNumber(targetRoomNum);
						if (!roomOpt)
						{
							Main::Details::sendMessage("Room not found.", session);
						}
						else
						{
							Main::Handlers::handleRoomJoin(response, session, roomsManager, !roomOpt->get().getPassword().empty());
						}
					}
					else
					{
						Main::Details::sendMessage("This friend is currently not inside a room.", session);
						return;
					}
				}
				else
				{
					Main::Details::sendMessage("The friend you're trying to join went offline.", session);
					return;
				}
			}
			else if (request.getExtra() == RoomInviteJoinExtra::USER_INVITING_FRIEND) // Currently doesn't work, something's wrong here
			{
				char targetNickname[16];
				std::ranges::copy(std::span(reinterpret_cast<const char*>(request.getData()), sizeof(targetNickname)), targetNickname);

				Main::Network::Session* targetSession = sessionsManager.findSessionByName(targetNickname);
				if (targetSession)
				{
					auto foundRoom = roomsManager.getRoomByNumber(session.getRoomNumber());
					if (foundRoom)
					{
						auto& actualRoom = foundRoom->get();
						response.setOrder(319); // Room invite request
						response.setExtra(44);  // Room invite request
						response.setOption(0);
						response.setSession(targetSession->getId());

#pragma pack(push, 1)
						struct Response
						{
							std::uint32_t serverId = 4; // unsure
							char sourceNickname[16]{};
							std::uint16_t roomNumber{};
							std::uint16_t unknown{ 2 }; // same as for RoomJoin structure, always 2
							char roomTitle[30]{};
							std::uint16_t padding = 0;
							char password[8]{};
							std::uint64_t padding2 = 0;
						};
#pragma pack(pop)

						Response responseStruct;
						responseStruct.roomNumber = session.getRoomNumber() - 1;
						std::ranges::copy(std::span(actualRoom.getRoomTitle().c_str(), sizeof(responseStruct.roomTitle) - 1), responseStruct.roomTitle);
						responseStruct.roomTitle[sizeof(responseStruct.roomTitle) - 1] = '\0';
						std::ranges::copy(std::span(session.getAccountInfo().nickname, sizeof(responseStruct.sourceNickname) - 1), responseStruct.sourceNickname);
						responseStruct.sourceNickname[sizeof(responseStruct.sourceNickname) - 1] = '\0';
						response.setData(reinterpret_cast<std::uint8_t*>(&responseStruct), sizeof(responseStruct));
						targetSession->asyncWrite(response);
					}
				}
				else
				{

				}
			}
			else if (session.getRoomNumber())
			{
				roomsManager.broadcastToRoom(session.getRoomNumber(), const_cast<Common::Network::Packet&>(request));
			}
		}

		// Takes care of settings that are inside the "Room Settings" button + switching team
		inline void handleRoomMiscellaneous(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager,
			std::uint64_t m_latestServerRestart)
		{
			Utils::Logger& logger = Utils::Logger::getInstance();

			auto foundRoom = roomsManager.getRoomByNumber(session.getRoomNumber());
			if (foundRoom == std::nullopt) return;
			auto& room = foundRoom.value().get();

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());

			if (request.getOrder() == 121 && request.getExtra() == 28) // votekick
			{
				// Unsure
				struct VotekickResponse
				{
					Main::Structures::UniqueId targetUniqueId; 
					Main::Structures::UniqueId selfUniqueId;
					std::uint32_t kickReasonId = 0;
					std::uint32_t totalVoteTime = 0; // here the tick is in ms / 10, because 4 bytes might not be enough for tick in pure milliseconds!
				};
				VotekickResponse votekickResponse;

				// OK reading client request
				std::memcpy(&votekickResponse.targetUniqueId, reinterpret_cast<std::uint8_t*>(const_cast<std::uint8_t*>(request.getData())), 
					sizeof(Main::Structures::UniqueId));

				const auto& accountInfo = session.getAccountInfo();
				votekickResponse.selfUniqueId = accountInfo.uniqueId;

				// Ok reading client request
				std::memcpy(&votekickResponse.kickReasonId, reinterpret_cast<std::uint8_t*>(const_cast<std::uint8_t*>(request.getData() + 4)),
					sizeof(std::uint32_t));

				const std::uint32_t totalVoteTime = 30'000; // in ms
				votekickResponse.totalVoteTime = (static_cast<std::uint32_t>(accountInfo.getUtcTimeMs() - m_latestServerRestart + totalVoteTime) / 10);

				response.setExtra(28);
				response.setMission(0);
				response.setOption(1);

				response.setData(reinterpret_cast<std::uint8_t*>(&votekickResponse), sizeof(votekickResponse));
				roomsManager.broadcastToRoom(session.getRoomNumber(), response);
				return;
			}
			else if (request.getMission() == 1 && request.getOrder() == 125) // add password without clicking "room settings" first
			{
				auto response = request;
				char newPassword[8]{};
				std::memcpy(newPassword, request.getData() + 4, sizeof(newPassword));
				room.updatePassword(newPassword);
				room.broadcastToRoom(response);

				logger.log("Password changed without clicking room settings", Utils::LogType::Normal, "Main::handleRoomMiscellaneous");
				return;
			}
			else if (request.getMission() == 0 && request.getOrder() == 125) // password removed without clicking "room settings" first
			{
				auto response = request;
				room.updatePassword("");
				room.broadcastToRoom(response);

				logger.log("Password removed without clicking room settings", Utils::LogType::Normal, "Main::handleRoomMiscellaneous");
				return;
			}

			if (request.getOrder() == 155) // team switch
			{
				response.setOrder(313); // Switch team response to client
				response.setExtra(1);   // Success -- Make sanity checks? Client already prevents this
				auto uniqueId = session.getAccountInfo().uniqueId;
				room.changePlayerTeam(session.getAccountInfo().uniqueId, request.getOption());
				response.setOption(request.getOption()); 
				response.setData(reinterpret_cast<std::uint8_t*>(&uniqueId), sizeof(uniqueId));
				room.broadcastToRoom(response);
				return;
			}

			response.setExtra(request.getExtra());
			response.setOption(request.getOption());
			response.setMission(request.getMission());

			const auto previousGameMode = room.getRoomSettings().mode;
			const bool wasPreviousGameModeTeamBased = room.isModeTeamBased();

			if (request.getDataSize() == sizeof(Main::Structures::RoomSettingsUpdateBase)) // Both title + password changed
			{
				logger.log("Room Settings: Settings Updated", Utils::LogType::Normal, "Main::handleRoomMiscellaneous");

				Main::Structures::RoomSettingsUpdateBase updatedRoomSettings;
				std::memcpy(&updatedRoomSettings, request.getData(), request.getDataSize());

				room.updateRoomSettings(updatedRoomSettings, request.getOption());
				if (request.getOrder() == 125)
				{
					room.updatePassword("");
				}

				// Disable team balance
				updatedRoomSettings.isTeamBalanceOn = 0;
				auto data = reinterpret_cast<std::uint8_t*>(&updatedRoomSettings);
				response.setData(reinterpret_cast<std::uint8_t*>(&updatedRoomSettings), request.getDataSize());
				room.broadcastToRoom(response);

			}
			else if (request.getDataSize() == sizeof(Main::Structures::RoomSettingsUpdateTitle))
			{
				logger.log("Room Settings: Settings Updated + Title update", Utils::LogType::Normal, "Main::handleRoomMiscellaneous");

				Main::Structures::RoomSettingsUpdateTitle updatedRoomSettings;
				std::memcpy(&updatedRoomSettings, request.getData(), request.getDataSize());

				room.updateRoomSettings(updatedRoomSettings.roomSettingsUpdateBase, request.getOption());
				room.updateTitle(updatedRoomSettings.title);

				// Disable team balance
				updatedRoomSettings.roomSettingsUpdateBase.isTeamBalanceOn = 0;
				auto data = reinterpret_cast<std::uint8_t*>(&updatedRoomSettings);
				response.setData(reinterpret_cast<std::uint8_t*>(&updatedRoomSettings), request.getDataSize());
				room.broadcastToRoom(response);
			}
			else if (request.getDataSize() == sizeof(Main::Structures::RoomSettingsUpdatePassword)) 
			{
				logger.log("Room Settings: Settings Updated + Password update", Utils::LogType::Normal, "Main::handleRoomMiscellaneous");

				Main::Structures::RoomSettingsUpdatePassword updatedRoomSettings;
				std::memcpy(&updatedRoomSettings, request.getData(), request.getDataSize());

				room.updateRoomSettings(updatedRoomSettings.roomSettingsUpdateBase, request.getOption());
				room.updatePassword(updatedRoomSettings.password);

				// Disable team balance
				updatedRoomSettings.roomSettingsUpdateBase.isTeamBalanceOn = 0;
				auto data = reinterpret_cast<std::uint8_t*>(&updatedRoomSettings);
				response.setData(reinterpret_cast<std::uint8_t*>(&updatedRoomSettings), request.getDataSize());
				room.broadcastToRoom(response);
			}
			else if (request.getDataSize() == sizeof(Main::Structures::RoomSettingsUpdateTitlePassword))
			{
				logger.log("Room Settings: Settings updated + Both Title + Password changed", Utils::LogType::Normal, "Main::handleRoomMiscellaneous");

				Main::Structures::RoomSettingsUpdateTitlePassword updatedRoomSettings;
				std::memcpy(&updatedRoomSettings, request.getData(), request.getDataSize());

				room.updateRoomSettings(updatedRoomSettings.roomSettingsUpdateBase, request.getOption());
				room.updatePassword(updatedRoomSettings.password);
				room.updateTitle(updatedRoomSettings.title);

				// Disable team balance
				updatedRoomSettings.roomSettingsUpdateBase.isTeamBalanceOn = 0;
				auto data = reinterpret_cast<std::uint8_t*>(&updatedRoomSettings);
				response.setData(reinterpret_cast<std::uint8_t*>(&updatedRoomSettings), request.getDataSize());
				room.broadcastToRoom(response);
			}

			const auto& newGameMode = room.getRoomSettings().mode;
			const bool isNewGameModeTeamBased = room.isModeTeamBased();

			if (previousGameMode != newGameMode) 
			{
				// Check whether we went from NonTeamBased => TeamBased.
				if (!wasPreviousGameModeTeamBased && isNewGameModeTeamBased)
				{
					logger.log("Room Settings: Mode changed from NonTeamBased => TeamBased. Re-disabling team balance...",
						Utils::LogType::Normal, "Main::handleRoomMiscellaneous");

					// Disable Team balance for now: it causes issues such as team bugs.
					response.setOrder(121);
					response.setMission(0);
					response.setExtra(0);
					response.setOption(room.getRoomSettings().mode);
					auto settings = room.getRoomSettingsUpdate();
					response.setData(reinterpret_cast<std::uint8_t*>(&settings), sizeof(settings));
					session.asyncWrite(response);

					room.updatePlayersTeamToTeamBased();

					logger.log("Room Settings: Updated player teams to room-based team",
						Utils::LogType::Normal, "Main::handleRoomMiscellaneous");
				}
				else if (wasPreviousGameModeTeamBased && !isNewGameModeTeamBased) // Otherwise, check whether we went from TeamBased => NonTeamBased
				{
					logger.log("Room Settings: Mode changed from TeamBased => NonTeamBased. Updating teams...",
						Utils::LogType::Normal, "Main::handleRoomMiscellaneous");

					room.updatePlayersTeamToNonTeamBased();

					logger.log("Room Settings: Updated player teams to room-based team",
						Utils::LogType::Normal, "Main::handleRoomMiscellaneous");
				}
			}
		}
	}
}

#endif