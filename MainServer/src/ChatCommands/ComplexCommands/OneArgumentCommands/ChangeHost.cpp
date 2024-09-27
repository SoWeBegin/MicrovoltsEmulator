
#include <string>
#include <charconv>
#include "../../../../include/Network/MainSession.h"
#include "../../../../include/Classes/RoomsManager.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ComplexCommands/OneArgumentCommands/AbstractOneArgNonNumericalRoomCommand.h"
#include "../../../../include/ChatCommands/ComplexCommands/OneArgumentCommands/ChangeHost.h"

namespace Main
{
	namespace Command
	{
		void ChangeHost::execute(const std::string& providedCommand, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager,
			 Common::Network::Packet& response)
		{
			if (!parseCommand(providedCommand))
			{
				this->m_confirmationMessage += "error: parsing error";
				sendConfirmation(response, session);
				return;
			}

			if (Main::Classes::Room* room = roomsManager.getRoomByNumber(session.getRoomNumber()))
			{
				this->m_confirmationMessage += "command under maintenance";
				sendConfirmation(response, session);
				return;

				const bool changed = false; //room->changeHostByNickname(m_value);
				if (!changed)
				{
					this->m_confirmationMessage += "error";
					sendConfirmation(response, session);
				}
				else
				{
					this->m_confirmationMessage += "success";
					sendConfirmation(response, session);
				}
			}
			else
			{
				this->m_confirmationMessage += "error: room not found";
				sendConfirmation(response, session);
			}
		}

		void ChangeHost::getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response)
		{
			this->m_confirmationMessage += "/changehost newHostName: assigns the host to the player identified by newHostName";
			sendConfirmation(response, session);
		}
	};
}
