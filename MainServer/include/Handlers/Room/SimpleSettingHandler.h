#ifndef ROOM_SETTINGS_SIMPLE_ROOM_SETTING_HANDLER_HEADER
#define ROOM_SETTINGS_SIMPLE_ROOM_SETTING_HANDLER_HEADER

#include "../../Network/MainSession.h"
#include "../../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "Network/Packet.h"
#include "../include/Classes/RoomsManager.h"

namespace Main
{
	namespace Handlers
	{
		// Takes care of settings that are outside the "Room Settings" button
		template<Main::Enums::RoomSimpleSetting T>
		inline void handleSimpleRoomSetting(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager)
		{
			Utils::Logger& logger = Utils::Logger::getInstance();

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());
			response.setOption(request.getOption());
			response.setMission(request.getMission());

			auto foundRoom = roomsManager.getRoomByNumber(session.getRoomNumber());
			if (foundRoom == std::nullopt) return;
			auto& room = foundRoom.value().get();

			if constexpr (T == Main::Enums::RoomSimpleSetting::SETTING_MAP)
			{
				response.setExtra(request.getExtra());
				response.setData(reinterpret_cast<std::uint8_t*>(const_cast<std::uint8_t*>(request.getData())), request.getDataSize());
				room.updateMap(request.getExtra());

				logger.log("The map has been updated", Utils::LogType::Normal, "Main::handleSimpleRoomSettings");
			} 
			else if constexpr (T == Main::Enums::RoomSimpleSetting::SETTING_PLAYERS_PER_TEAM)
			{
				room.setPlayersPerTeam(request.getOption() / 2); // for square mode where 1 player is chosen, this results in 0 and is taken care elsewhere!
				logger.log("PlayersPerTeam has been updated", Utils::LogType::Normal, "Main::handleSimpleRoomSettings");
			}
			else if constexpr (T == Main::Enums::RoomSimpleSetting::SETTING_TIME)
			{
				room.setTime(request.getOption());
				logger.log("Total match time has been updated", Utils::LogType::Normal, "Main::handleSimpleRoomSettings");
			}
			else if constexpr (T == Main::Enums::RoomSimpleSetting::SETTING_SPECIFIC)
			{
				room.setSpecificSetting(request.getOption());
				logger.log("Specific setting has been updated", Utils::LogType::Normal, "Main::handleSimpleRoomSettings");
			}
			else
			{
				if constexpr (T == Main::Enums::SETTING_TEAMBALANCE)
				{
					return; // For now, team balance is not supported: it causes issues such as the team bug.
				}

				room.switchSimpleSetting<T>();

				if constexpr (T == Main::Enums::SETTING_ITEM)
					logger.log("Item setting has been updated", Utils::LogType::Normal, "Main::handleSimpleRoomSettings");
				else if constexpr (T == Main::Enums::SETTING_OPEN)
					logger.log("Open setting has been updated", Utils::LogType::Normal, "Main::handleSimpleRoomSettings");
				else if constexpr (T == Main::Enums::SETTING_TEAMBALANCE)
					logger.log("Team balance has been updated", Utils::LogType::Normal, "Main::handleSimpleRoomSettings");
				else if constexpr (T == Main::Enums::SETTING_OBSERVER)
					logger.log("Observer Mode setting has been updated", Utils::LogType::Normal, "Main::handleSimpleRoomSettings");
			}
			room.broadcastToRoom(response);
		}
	}
}

#endif