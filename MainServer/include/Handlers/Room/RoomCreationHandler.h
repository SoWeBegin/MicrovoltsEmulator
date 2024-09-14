#ifndef ROOM_CREATION_HANDLER_H
#define ROOM_CREATION_HANDLER_H

#include "../../Network/MainSession.h"
#include "Network/Packet.h"
#include "../../MainEnums.h"
#include "../../Structures/Room/ClientRoomCreationInfo.h"

#include "../../Classes/RoomsManager.h"
#include "../../Classes/Room.h"
#include <Utils/Logger.h>

namespace Main
{
	namespace Handlers
	{
		enum RoomCreationExtra
		{
			CREATION_FAIL = 0,
			CREATION_SUCCESS = 1,
			CREATION_EMPTY = 6,
			CREATION_BOTBATTLE = 61,
		};

		// Checked
		inline void handleRoomCreation(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager)
		{
			if (request.getFullSize() == sizeof(Common::Protocol::TcpHeader) + sizeof(Common::Protocol::CommandHeader)) return;

			const auto& selfAccountInfo = session.getAccountInfo();

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());

			Main::Structures::RoomPlayerInfo roomCreator{};
			roomCreator.character = selfAccountInfo.latestSelectedCharacter;
			roomCreator.level = selfAccountInfo.playerLevel;
			std::memcpy(roomCreator.playerName, selfAccountInfo.nickname, 16);
			roomCreator.state = Common::Enums::STATE_NORMAL;
			roomCreator.uniqueId = selfAccountInfo.uniqueId;
			roomCreator.ping = session.getPing();
			roomCreator.team = Common::Enums::TEAM_BLUE;

			// request.getOption() ==> server/channel ID
			Main::Structures::CompleteRoomInfo completeRoomInfo;
			std::memcpy(&completeRoomInfo, request.getData(), sizeof(completeRoomInfo));

			Main::Classes::Room room{ completeRoomInfo.title, completeRoomInfo.roomSettings, roomCreator, &session };
			if (request.getDataSize() >= sizeof(Main::Structures::CompleteRoomInfo))
			{
				room.setPassword(completeRoomInfo.password);
			}
			room.setSpecificSetting(request.getExtra()); // request.getExtra() == specific setting. E.g eli => num of rounds, TDM => num of total kills / 10
			
			struct ResponseMessage
			{
				std::uint16_t roomNumber{};
				std::uint16_t unknown{};
			} responseMessage;

			Utils::Logger& logger = Utils::Logger::getInstance();

			if (roomsManager.getTotalRooms() >= 30)
			{
				response.setExtra(RoomCreationExtra::CREATION_FAIL);
				session.asyncWrite(response);

				logger.log("The player " + session.getPlayerInfoAsString() + " attempted to create a new room, but there are >= 30 rooms thus the creation failed. " 
					+ room.getRoomInfoAsString(), Utils::LogType::Warning, "Main::handleRoomCreation");
			}
			else
			{
				roomsManager.addRoom(room);

				response.setExtra(RoomCreationExtra::CREATION_SUCCESS);
				responseMessage.roomNumber = room.getRoomNumber() - 1;
				response.setData(reinterpret_cast<std::uint8_t*>(&responseMessage), sizeof(responseMessage));
				session.asyncWrite(response);
				room.setStateFor(session.getAccountInfo().uniqueId, Common::Enums::STATE_WAITING);
				session.setRoomNumber(room.getRoomNumber());
				session.setIsInLobby(false);


				// Disable Team balance for now: it causes issues such as team bugs.
				if (room.isModeTeamBased())
				{
					response.setOrder(125);
					response.setMission(0);
					response.setExtra(0);
					response.setOption(room.getRoomSettings().mode);
					auto settings = room.getRoomSettingsUpdate();
					response.setData(reinterpret_cast<std::uint8_t*>(&settings), sizeof(settings));
					session.asyncWrite(response);
				}

				logger.log("The player " + session.getPlayerInfoAsString() + " has created a new room. " + room.getRoomInfoAsString(),
					Utils::LogType::Normal, "Main::handleRoomCreation");
			}
		}
	}
}

#endif