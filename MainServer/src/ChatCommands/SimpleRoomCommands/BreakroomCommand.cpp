
#include <string>
#include <charconv>
#include "../../../include/Network/MainSession.h"
#include "../../../include/Classes/RoomsManager.h"
#include "../../../include/ChatCommands/SimpleRoomCommands/BreakroomCommand.h"
#include "../../../include/Classes/Room.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ISimpleRoomCommand.h"

#include <chrono> 

namespace Main
{
	namespace Command
	{
		Breakroom::Breakroom(const Common::Enums::PlayerGrade requiredGrade)
			: ISimpleRoomCommand{ requiredGrade }
		{
		}

		void Breakroom::execute(Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager, std::size_t roomNumber, Common::Network::Packet& response)
		{
			this->m_confirmationMessage += "success";
			response.setOrder(137);
			response.setExtra(0x1B);

			if (Main::Classes::Room* room = roomsManager.getRoomByNumber(roomNumber))
			{
				// kick whole room
				room->broadcastToRoom(response);

				// notify room
				response.setOrder(316);
				response.setExtra(1);
				response.setMission(0);
				std::string message = std::string(16, '0');
				message += "The room has been closed by a MOD/GM";
				response.setData(reinterpret_cast<std::uint8_t*>(message.data()), message.size());
				response.setOption(message.size());
				room->broadcastToRoom(response);

				// delete the room
				room->breakroom();
				roomsManager.removeRoom(roomNumber);
			}
			else
			{
				this->m_confirmationMessage += "error";
			}
			sendConfirmation(response, session);
		}

		void Breakroom::getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response)
		{
			this->m_confirmationMessage += "/breakroom: kicks all players from the room and closes it";
			sendConfirmation(response, session);
		}
	};
}