#ifndef MUTEINFO_COMPLEXCMD_HEADER
#define MUTEINFO_COMPLEXCMD_HEADER

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
		struct MuteInfo : public Common::Command::AbstractOneArgNonNumericalCommand<Main::Network::Session, Main::Network::SessionsManager>
		{
			MuteInfo(const Common::Enums::PlayerGrade requiredGrade)
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
					this->m_confirmationMessage += "The player is offline.";
					sendConfirmation(response, session);
					return;
				}
				if (!targetSession->isMuted())
				{
					this->m_confirmationMessage += "The player is not currently muted.";
					sendConfirmation(response, session);
					return;
				}

				auto muteInfo = targetSession->getMuteInfo();
				if (muteInfo.mutedUntil.starts_with("-"))
				{
					muteInfo.mutedUntil = "forever";
				}
				this->m_confirmationMessage += "Muted by: ";
				this->m_confirmationMessage += muteInfo.mutedBy;
				this->m_confirmationMessage += ", reason: ";
				this->m_confirmationMessage += muteInfo.reason;
				this->m_confirmationMessage += ", until: ";
				this->m_confirmationMessage += muteInfo.mutedUntil;
				sendConfirmation(response, session);
			}

			void getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response) override
			{
				this->m_confirmationMessage += "/muteinfo: provides reason, expiration date and the nickname of whoever the mute";
				sendConfirmation(response, session);
			}

		};
	}
}
#endif