#ifndef MUTEPLAYER_COMPLEXCOMMAND_HEADER
#define MUTEPLAYER_COMPLEXCOMMAND_HEADER

#include <string>
#include <charconv>
#include "../../../Network/MainSession.h"
#include "../../../Network/MainSessionManager.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ComplexCommands/ThreeArgumentCommands/AbstractThreeArgCommand.h"

namespace Main
{
	namespace Command
	{
		struct MutePlayer : public Common::Command::AbstractThreeArgCommand<Main::Network::Session, Main::Network::SessionsManager>
		{
			MutePlayer(const Common::Enums::PlayerGrade requiredGrade)
				: AbstractThreeArgCommand<Main::Network::Session, Main::Network::SessionsManager>{ requiredGrade }
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

				auto* targetSession = sessionManager.findSessionByName(m_playername.c_str());
				if (!targetSession)
				{
					this->m_confirmationMessage += "The player is offline. Use /muteoffline to mute offline players.";
					sendConfirmation(response, session);
					return;
				}
				if (targetSession->getAccountInfo().playerGrade >= session.getAccountInfo().playerGrade)
				{
					this->m_confirmationMessage += "Error: The target user has equal or greater grade than you.";
					sendConfirmation(response, session);
					return;
				}

				targetSession->muteAccount(this->m_duration, this->m_furtherInfo, session.getAccountInfo().nickname);

				this->m_confirmationMessage += "success";
				sendConfirmation(response, session);
			}

			void getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response) override
			{
				this->m_confirmationMessage += "/mute nickname duration(in days) reason: mutes an online player identified by nickname, for a given duration (in days) and reason";
				sendConfirmation(response, session);
			}

		};
	}
}
#endif