#ifndef TILT_CLIENT_COMPLEXCOMMAND_HEADER
#define TILT_CLIENT_COMPLEXCOMMAND_HEADER

#include <string>
#include <charconv>
#include "../../../Network/MainSession.h"
#include "../../../Network/MainSessionManager.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ComplexCommands/OneArgumentCommands/AbstractOneArgNonNumericalCommand.h"


namespace Main
{
	namespace Command
	{
		struct TiltClient : public Common::Command::AbstractOneArgNonNumericalCommand<Main::Network::Session, Main::Network::SessionsManager>
		{
			TiltClient(const Common::Enums::PlayerGrade requiredGrade);

			void execute(const std::string& providedCommand, Main::Network::Session& session, Main::Network::SessionsManager& sessionManager, Common::Network::Packet& response) override;

			void getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response) override;
		};
	}
}
#endif