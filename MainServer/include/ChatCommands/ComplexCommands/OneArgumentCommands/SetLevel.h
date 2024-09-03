#ifndef SETLEVEL_COMPLEXCOMMAND_HEADER
#define SETLEVEL_COMPLEXCOMMAND_HEADER

#include <string>
#include <charconv>
#include "../../../Network/MainSession.h"
#include "../../../Network/MainSessionManager.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ComplexCommands/OneArgumentCommands/AbstractOneArgNumericalCommand.h"


namespace Main
{
	namespace Command
	{
		// Todo: Execution of this command unequips all equipped items, fix that.
		struct SetLevel : public Common::Command::AbstractOneArgNumericalCommand<Main::Network::Session, Main::Network::SessionsManager>
		{
			SetLevel(const Common::Enums::PlayerGrade requiredGrade);

			void execute(const std::string& providedCommand, Main::Network::Session& session, Main::Network::SessionsManager& sessionManager, Common::Network::Packet& response) override;

			void getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response) override;
		};
	}
}
#endif