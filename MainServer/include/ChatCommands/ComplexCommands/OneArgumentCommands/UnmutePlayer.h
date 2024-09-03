#ifndef UNMUTEPLAYER_COMPLEXCOMMAND_HEADER
#define UNMUTEPLAYER_COMPLEXCOMMAND_HEADER

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
		struct UnmutePlayer : public Common::Command::AbstractOneArgNonNumericalCommand<Main::Network::Session, Main::Network::SessionsManager>
		{
			UnmutePlayer(const Common::Enums::PlayerGrade requiredGrade)
				: AbstractOneArgNonNumericalCommand<Main::Network::Session, Main::Network::SessionsManager>{ requiredGrade }
			{
			}

			void execute(const std::string& providedCommand, Main::Network::Session& session, Main::Network::SessionsManager& sessionManager,
				Common::Network::Packet& response) override
			{
				if (!parseCommand(providedCommand))
				{
					this->m_confirmationMessage += "error: parsing error";
					sendConfirmation(response, session);
					return;
				}

				auto* targetSession = sessionManager.findSessionByName(m_value.c_str());
				if (!targetSession)
				{
					this->m_confirmationMessage += "The player is offline. Use /unmuteoffline to unmute offline players.";
					sendConfirmation(response, session);
					return;
				}

				targetSession->unmuteAccount();

				this->m_confirmationMessage += "success";
				sendConfirmation(response, session);
			}

			void getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response) override
			{
				this->m_confirmationMessage += "/unmute nickname: unmutes an online player identified by nickname that was previously muted";
				sendConfirmation(response, session);
			}

		};
	}
}
#endif