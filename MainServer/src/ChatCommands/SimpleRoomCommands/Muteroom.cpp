
#include <string>
#include <charconv>
#include "../../../include/Network/MainSession.h"
#include "../../../include/Classes/RoomsManager.h"
#include "../../../include/ChatCommands/SimpleRoomCommands/Muteroom.h"
#include "../../../include/Classes/Room.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ISimpleRoomCommand.h"

#include <boost/interprocess/shared_memory_object.hpp> 
#include <boost/interprocess/mapped_region.hpp> 
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
			this->m_confirmationMessage += "success";

			auto foundRoom = roomsManager.getRoomByNumber(roomNumber);
			if (foundRoom == std::nullopt)
			{
				this->m_confirmationMessage += "error";
			}
			else
			{
				auto& room = foundRoom.value().get();
				room.muteRoom();
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