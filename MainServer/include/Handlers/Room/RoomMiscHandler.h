#ifndef ROOM_MISC_HANDLER_HEADER
#define ROOM_MISC_HANDLER_HEADER

#include "../../Network/MainSession.h"
#include "../../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "../../../include/Structures/Room/RoomSettingsUpdate.h"
#include "Network/Packet.h"
#include "../Room/RoomJoinHandler.h"
#include "../Room/RoomLeaveHandler.h"


namespace Main
{
	namespace Handlers
	{
		// Takes care of settings that are inside the "Room Settings" button + switching team
		inline void handleRoomMiscellaneous(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager,
			std::uint64_t m_latestServerRestart)
		{
			if (Main::Classes::Room* room = roomsManager.getRoomByNumber(session.getRoomNumber()))
			{
				auto response = request;
				if (request.getOrder() == 121 && request.getExtra() == 28) // votekick
				{
					struct VotekickResponse
					{
						Main::Structures::UniqueId targetUniqueId{};
						Main::Structures::UniqueId selfUniqueId{};
						std::uint32_t kickReasonId = 0;
						std::uint32_t totalVoteTime = 0; // here the tick is in ms / 10, because 4 bytes might not be enough for tick in pure milliseconds!
					} votekickResponse;

					// targetUniqueId and kickReason is provided by the client
					std::memcpy(&votekickResponse.targetUniqueId, reinterpret_cast<std::uint8_t*>(const_cast<std::uint8_t*>(request.getData())),
						sizeof(Main::Structures::UniqueId));
					std::memcpy(&votekickResponse.kickReasonId, reinterpret_cast<std::uint8_t*>(const_cast<std::uint8_t*>(request.getData() + 4)),
						sizeof(std::uint32_t));

					const auto& accountInfo = session.getAccountInfo();
					const std::uint32_t totalVoteTime = 30'000; // in ms
					votekickResponse.selfUniqueId = accountInfo.uniqueId;
					votekickResponse.totalVoteTime = (static_cast<std::uint32_t>(accountInfo.getUtcTimeMs() - m_latestServerRestart + totalVoteTime) / 10);

					response.setCommand(request.getOrder(), 0, 28, 1);
					response.setData(reinterpret_cast<std::uint8_t*>(&votekickResponse), sizeof(votekickResponse));
					roomsManager.broadcastToRoom(session.getRoomNumber(), response);
					return;
				}
				else if (request.getMission() == 1 && request.getOrder() == 125) // add password without clicking "room settings" first
				{
					char newPassword[8]{};
					std::memcpy(newPassword, request.getData() + 4, sizeof(newPassword));
					room->updatePassword(newPassword);
					room->broadcastToRoom(response);
					return;
				}
				else if (request.getMission() == 0 && request.getOrder() == 125) // password removed without clicking "room settings" first
				{
					room->updatePassword("");
					room->broadcastToRoom(response);
					return;
				}
				else if (request.getOrder() == 155) // team switch
				{
					response.setOrder(313); // Switch team response to client
					response.setExtra(1);   // Success -- Make sanity checks? Client already prevents this
					auto uniqueId = session.getAccountInfo().uniqueId;
					response.setOption(request.getOption());
					response.setData(reinterpret_cast<std::uint8_t*>(&uniqueId), sizeof(uniqueId));
					room->changePlayerTeam(session.getAccountInfo().uniqueId, request.getOption());
					room->broadcastToRoom(response);
					return;
				}
				else
				{
					if (request.getDataSize() == sizeof(Main::Structures::RoomSettingsUpdateBase))
					{
						Main::Structures::RoomSettingsUpdateBase updatedRoomSettings;
						std::memcpy(&updatedRoomSettings, request.getData(), request.getDataSize());

						room->updateRoomSettings(updatedRoomSettings, request.getOption());
						if (request.getOrder() == 125)
						{
							room->updatePassword("");
						}

						// Disable team balance
						updatedRoomSettings.isTeamBalanceOn = 0;
						auto data = reinterpret_cast<std::uint8_t*>(&updatedRoomSettings);
						response.setData(reinterpret_cast<std::uint8_t*>(&updatedRoomSettings), request.getDataSize());
						room->broadcastToRoom(response);
					}
					else if (request.getDataSize() == sizeof(Main::Structures::RoomSettingsUpdateTitle))
					{
						Main::Structures::RoomSettingsUpdateTitle updatedRoomSettings;
						std::memcpy(&updatedRoomSettings, request.getData(), request.getDataSize());

						room->updateRoomSettings(updatedRoomSettings.roomSettingsUpdateBase, request.getOption());
						room->updateTitle(updatedRoomSettings.title);

						// Disable team balance
						updatedRoomSettings.roomSettingsUpdateBase.isTeamBalanceOn = 0;
						auto data = reinterpret_cast<std::uint8_t*>(&updatedRoomSettings);
						response.setData(reinterpret_cast<std::uint8_t*>(&updatedRoomSettings), request.getDataSize());
						room->broadcastToRoom(response);
					}
					else if (request.getDataSize() == sizeof(Main::Structures::RoomSettingsUpdatePassword))
					{
						Main::Structures::RoomSettingsUpdatePassword updatedRoomSettings;
						std::memcpy(&updatedRoomSettings, request.getData(), request.getDataSize());

						room->updateRoomSettings(updatedRoomSettings.roomSettingsUpdateBase, request.getOption());
						room->updatePassword(updatedRoomSettings.password);

						// Disable team balance
						updatedRoomSettings.roomSettingsUpdateBase.isTeamBalanceOn = 0;
						auto data = reinterpret_cast<std::uint8_t*>(&updatedRoomSettings);
						response.setData(reinterpret_cast<std::uint8_t*>(&updatedRoomSettings), request.getDataSize());
						room->broadcastToRoom(response);
					}
					else if (request.getDataSize() == sizeof(Main::Structures::RoomSettingsUpdateTitlePassword))
					{
						Main::Structures::RoomSettingsUpdateTitlePassword updatedRoomSettings;
						std::memcpy(&updatedRoomSettings, request.getData(), request.getDataSize());

						room->updateRoomSettings(updatedRoomSettings.roomSettingsUpdateBase, request.getOption());
						room->updatePassword(updatedRoomSettings.password);
						room->updateTitle(updatedRoomSettings.title);

						// Disable team balance
						updatedRoomSettings.roomSettingsUpdateBase.isTeamBalanceOn = 0;
						auto data = reinterpret_cast<std::uint8_t*>(&updatedRoomSettings);
						response.setData(reinterpret_cast<std::uint8_t*>(&updatedRoomSettings), request.getDataSize());
						room->broadcastToRoom(response);
					}

					const auto& newGameMode = room->getRoomSettings().mode;
					const bool isNewGameModeTeamBased = room->isModeTeamBased();
					const auto previousGameMode = room->getRoomSettings().mode;
					const bool wasPreviousGameModeTeamBased = room->isModeTeamBased();

					if (previousGameMode != newGameMode)
					{
						if (!wasPreviousGameModeTeamBased && isNewGameModeTeamBased) // Check whether we went from NonTeamBased => TeamBased.
						{
							// Disable Team balance for now: it causes issues such as team bugs.
							response.setOrder(121);
							response.setMission(0);
							response.setExtra(0);
							response.setOption(room->getRoomSettings().mode);
							auto settings = room->getRoomSettingsUpdate();
							response.setData(reinterpret_cast<std::uint8_t*>(&settings), sizeof(settings));
							session.asyncWrite(response);

							room->updatePlayersTeamToTeamBased();
						}
						else if (wasPreviousGameModeTeamBased && !isNewGameModeTeamBased) // Otherwise, check whether we went from TeamBased => NonTeamBased
						{
							room->updatePlayersTeamToNonTeamBased();
						}
					}
				}
			}
		}
	}
}

#endif