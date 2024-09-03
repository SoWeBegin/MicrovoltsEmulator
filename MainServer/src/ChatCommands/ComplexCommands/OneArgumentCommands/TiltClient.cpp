

#include <string>
#include <charconv>
#include "../../../../include/Network/MainSession.h"
#include "../../../../include/Network/MainSessionManager.h"
#include "../../../../include/ChatCommands/ComplexCommands/OneArgumentCommands/TiltClient.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ComplexCommands/OneArgumentCommands/AbstractOneArgNonNumericalCommand.h"


namespace Main
{
	namespace Command
	{
		TiltClient::TiltClient(const Common::Enums::PlayerGrade requiredGrade)
			: AbstractOneArgNonNumericalCommand<Main::Network::Session, Main::Network::SessionsManager>{ requiredGrade }
		{
		}

		void TiltClient::execute(const std::string& providedCommand, Main::Network::Session& session, Main::Network::SessionsManager& sessionManager, Common::Network::Packet& response)
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
			if (targetSession->getAccountInfo().playerGrade >= session.getAccountInfo().playerGrade)
			{
				this->m_confirmationMessage += "Error: The target user has equal or greater grade than you.";
				sendConfirmation(response, session);
				return;
			}
			sessionManager.removeSession(targetSession->getId());
			response.setOrder(73);
			response.setExtra(0x23);
			targetSession->asyncWrite(response);
			this->m_confirmationMessage += "The player was disconnected";
			sendConfirmation(response, session);
			return;
		}

		void TiltClient::getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response)
		{
			this->m_confirmationMessage += "/disconnect nickname: disconnects the client of the user specified by nickname";
			sendConfirmation(response, session);
		}

	};
}
