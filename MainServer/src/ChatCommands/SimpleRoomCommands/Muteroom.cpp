
#include <string>
#include <charconv>
#include "../../../include/Network/MainSession.h"
#include "../../../include/Classes/RoomsManager.h"
#include "../../../include/ChatCommands/SimpleRoomCommands/Muteroom.h"
#include "../../../include/Classes/Room.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ISimpleRoomCommand.h"

#include <chrono> 

namespace Main
{
	namespace Command
	{
		Muteroom::Muteroom(const Common::Enums::PlayerGrade requiredGrade)
			: ISimpleRoomCommand{ requiredGrade }
		{
		}

		void Muteroom::execute(Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager, std::size_t roomNumber, Common::Network::Packet& response)
		{
			if (Main::Classes::Room* room = roomsManager.getRoomByNumber(roomNumber))
			{
				room->muteRoom();
				this->m_confirmationMessage += "success";
			}
			else
			{
				this->m_confirmationMessage += "error";
			}
			sendConfirmation(response, session);
		}

		void Muteroom::getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response)
		{
			this->m_confirmationMessage += "/muteroom: mutes the whole room";
			sendConfirmation(response, session);
		}
	};
}