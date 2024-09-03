#ifndef UNBANPLAYEROFFLINE_COMPLEXCOMMAND_HEADER
#define UNBANPLAYEROFFLINE_COMPLEXCOMMAND_HEADER

#include <string>
#include <charconv>
#include "../../Network/MainSession.h"
#include "../../Network/MainSessionManager.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/DatabaseCommands//AbstractOneArgNonNumericalDbCommand.h"

namespace Main
{
	namespace Command
	{
		struct UnbanAccount : public Common::Command::AbstractOneArgNonNumericalDbCommand<Main::Network::Session, Main::Persistence::PersistentDatabase>
		{
			UnbanAccount(const Common::Enums::PlayerGrade requiredGrade);

			void execute(const std::string& providedCommand, Main::Network::Session& session, Main::Persistence::PersistentDatabase& database,
				Common::Network::Packet& response) override;

			void getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response) override;
		};
	}
}
#endif