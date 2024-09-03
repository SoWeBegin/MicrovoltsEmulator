#ifndef ADD_PLAYER_COMMAND_HEADER
#define ADD_PLAYER_COMMAND_HEADER

#include <string>
#include <charconv>
#include "../../Network/MainSession.h"
#include "../../Network/MainSessionManager.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/DatabaseCommands//AbstractThreeArgCommandDb.h"

namespace Main
{
	namespace Command
	{
		struct AddPlayer : public Common::Command::AbstractThreeArgCommandDb<Main::Network::Session, Main::Persistence::PersistentDatabase>
		{
			AddPlayer(const Common::Enums::PlayerGrade requiredGrade);

			void execute(const std::string& providedCommand, Main::Network::Session& session, Main::Persistence::PersistentDatabase& database,
				Common::Network::Packet& response) override;

			void getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response) override;
		};
	}
}
#endif