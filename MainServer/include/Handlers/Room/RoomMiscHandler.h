#ifndef ROOM_MISC_HANDLER_HEADER
#define ROOM_MISC_HANDLER_HEADER

#include "../../Network/MainSession.h"
#include "../../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "../../../include/Structures/Room/RoomSettingsUpdate.h"
#include "Network/Packet.h"

namespace Main
{
	namespace Handlers
	{
		inline void unknown(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager)
		{
			if (session.getRoomNumber())
			{
				roomsManager.broadcastToRoom(session.getRoomNumber(), const_cast<Common::Network::Packet&>(request));
			}
		}

		// Takes care of settings that are inside the "Room Settings" button + switching team
		inline void handleRoomMiscellaneous(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager)
		{
			Utils::Logger& logger = Utils::Logger::getInstance();

			auto foundRoom = roomsManager.getRoomByNumber(session.getRoomNumber());
			if (foundRoom == std::nullopt) return;
			auto& room = foundRoom.value().get();

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());

			if (request.getMission() == 2) // add password without clicking "room settings" first
			{
				auto response = request;
				char newPassword[8]{};
				std::memcpy(newPassword, request.getData(), 8);
				room.updatePassword(newPassword);
				room.broadcastToRoom(response);

				logger.log("Password changed without clicking room settings", Utils::LogType::Normal, "Main::handleRoomMiscellaneous");
				return;
			}
			else if (request.getMission() == 3) // password removed without clicking "room settings" first
			{
				auto response = request;
				room.updatePassword("");
				room.broadcastToRoom(response);

				logger.log("Password removed without clicking room settings", Utils::LogType::Normal, "Main::handleRoomMiscellaneous");
				return;
			}

			if (request.getOrder() == 159) // team switch
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
					response.setOrder(125);
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