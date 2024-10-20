
#include <string>
#include <charconv>
#include "../../../../include/Network/MainSession.h"
#include "../../../../include/Classes/RoomsManager.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ComplexCommands/OneArgumentCommands/AbstractOneArgNonNumericalRoomCommand.h"
#include "../../../../include/ChatCommands/ComplexCommands/OneArgumentCommands/Kick.h"


namespace Main
{
	namespace Command
	{
		KickPlayer::KickPlayer(const Common::Enums::PlayerGrade requiredGrade)
			: AbstractOneArgNonNumericalRoomCommand{ requiredGrade }
		{
		}

		void KickPlayer::execute(const std::string& providedCommand, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager,
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
				if (!room->kickPlayer(m_value.c_str()))
				{
					this->m_confirmationMessage += "Error: the player is either not in the room or they're >= MOD grade.";
					sendConfirmation(response, session);
					return;
				}
				else
				{
					this->m_confirmationMessage += "success";
					sendConfirmation(response, session);
				}
			}
			else
			{
				this->m_confirmationMessage += "You must be inside a room to kick a player.";
				sendConfirmation(response, session);
				return;
			}
		}

		void KickPlayer::getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response)
		{
			this->m_confirmationMessage += "/kick playerName: kicks the player identified by playerName from the current room";
			sendConfirmation(response, session);
		}

	};
}

