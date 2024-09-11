#ifndef CHANGEHOST_COMPLEXROOM_COMMAND_H
#define CHANGEHOST_COMPLEXROOM_COMMAND_H

#include <string>
#include <charconv>
#include "../../../Network/MainSession.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ComplexCommands/OneArgumentCommands/AbstractOneArgNonNumericalRoomCommand.h"
#include "../../../Classes/RoomsManager.h"


namespace Main
{
	namespace Command
	{
		struct ChangeHost : public Common::Command::AbstractOneArgNonNumericalRoomCommand<Main::Network::Session, Main::Classes::RoomsManager>
		{
			ChangeHost(const Common::Enums::PlayerGrade requiredGrade)
				: AbstractOneArgNonNumericalRoomCommand<Main::Network::Session, Main::Classes::RoomsManager>{requiredGrade}
			{
			}

			void execute(const std::string& providedCommand, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager,
			    Common::Network::Packet& response) override;

			void getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response) override;
		};
	}
}
#endif