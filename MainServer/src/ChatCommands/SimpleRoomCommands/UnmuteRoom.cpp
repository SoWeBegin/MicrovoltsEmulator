
#include <string>
#include <charconv>
#include "../../../include/Network/MainSession.h"
#include "../../../include/Classes/RoomsManager.h"
#include "../../../include/ChatCommands/SimpleRoomCommands/UnmuteRoom.h"
#include "../../../include/Classes/Room.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ISimpleRoomCommand.h"

#include <chrono> 

namespace Main
{
	namespace Command
	{
		UnmuteRoom::UnmuteRoom(const Common::Enums::PlayerGrade requiredGrade)
			: ISimpleRoomCommand{ requiredGrade }
		{
		}

		void UnmuteRoom::execute(Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager, std::size_t roomNumber, Common::Network::Packet& response)
		{
			this->m_confirmationMessage += "success";

			auto foundRoom = roomsManager.getRoomByNumber(roomNumber);
			if (foundRoom == std::nullopt)
			{
				this->m_confirmationMessage += "error";
			}
			else
			{
				auto& room = foundRoom.value().get();
				room.unmuteRoom();
			}
			sendConfirmation(response, session);
		}

		void UnmuteRoom::getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response)
		{
			this->m_confirmationMessage += "/unmuteroom: unmutes the room";
			sendConfirmation(response, session);
		}
	};
}