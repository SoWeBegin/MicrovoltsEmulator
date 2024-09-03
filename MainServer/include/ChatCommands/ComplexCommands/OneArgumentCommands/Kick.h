#ifndef KICKPLAYER_COMPLEXCOMMAND_HEADER
#define KICKPLAYER_COMPLEXCOMMAND_HEADER

#include <string>
#include "../../../Network/MainSession.h"
#include "../../../Classes/RoomsManager.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ComplexCommands/OneArgumentCommands/AbstractOneArgNonNumericalRoomCommand.h"


namespace Main
{
	namespace Command
	{
		struct KickPlayer : public Common::Command::AbstractOneArgNonNumericalRoomCommand<Main::Network::Session, Main::Classes::RoomsManager>
		{
			KickPlayer(const Common::Enums::PlayerGrade requiredGrade);

			void execute(const std::string& providedCommand, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager,
				Common::Network::Packet& response) override;

			void getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response) override;
		};
	}
}
#endif