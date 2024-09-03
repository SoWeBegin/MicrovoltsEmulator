#ifndef BANPLAYER_COMPLEXCOMMAND_HEADER
#define BANPLAYER_COMPLEXCOMMAND_HEADER

#include <string>
#include <charconv>
#include "../../../Network/MainSession.h"
#include "../../../Network/MainSessionManager.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ComplexCommands/ThreeArgumentCommands/AbstractThreeArgCommand.h"

// Don't divide this into a .cpp, linker errors to solve first!
namespace Main
{
	namespace Command
	{
		struct BanPlayer : public Common::Command::AbstractThreeArgCommand<Main::Network::Session, Main::Network::SessionsManager>
		{
			BanPlayer(const Common::Enums::PlayerGrade requiredGrade)
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
					this->m_confirmationMessage += "The player is offline. Use /banoffline to ban offline players.";
					sendConfirmation(response, session);
					return;
				}
				if (targetSession->getAccountInfo().playerGrade >= session.getAccountInfo().playerGrade)
				{
					this->m_confirmationMessage += "Error: The target user has equal or greater grade than you.";
					sendConfirmation(response, session);
					return;
				}

				sessionManager.removeSession(targetSession->getId());
				targetSession->banAccount(this->m_duration, this->m_furtherInfo, response);
				sessionManager.removeSession(targetSession->getId());
				this->m_confirmationMessage += "The player was banned.";
				sendConfirmation(response, session);
			}

			void getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response) override
			{
				this->m_confirmationMessage += "/ban nickname duration(in days) reason: bans an online player identified by nickname, for a given duration (in days) and reason";
				sendConfirmation(response, session);
			}

		};
	}
}
#endif