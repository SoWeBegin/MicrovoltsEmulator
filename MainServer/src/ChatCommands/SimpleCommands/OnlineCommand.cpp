
#include <string>
#include <charconv>
#include "../../../include/Network/MainSession.h"
#include "../../../include/Network/MainSessionManager.h"
#include "../../../include/ChatCommands/SimpleCommands/OnlineCommand.h"
#include "Enums/PlayerEnums.h"

#include "ChatCommands/ISimpleCommand.h"

namespace Main
{
	namespace Command
	{
		// Todo: Execution of this command unequips all equipped items, fix that.
		OnlineCommand::OnlineCommand(const Common::Enums::PlayerGrade requiredGrade)
			: ISimpleCommand{ requiredGrade }
		{
		}

		void OnlineCommand::execute(Main::Network::Session& session, Main::Network::SessionsManager& sessionManager, Common::Network::Packet& response)
		{
			constexpr std::size_t MAX_PACKET_SIZE = 1440;
			const auto& allSessions = sessionManager.getAllSessions();
			this->m_confirmationMessage += "Online players: ";
			this->m_confirmationMessage += std::to_string(allSessions.size());
			sendConfirmation(response, session);
			response.setOrder(316);
			response.setExtra(1);
			for (const auto& currentSession : sessionManager.getAllSessions())
			{
				const auto& accountInfo = currentSession.second->getAccountInfo();
				this->m_confirmationMessage += "- ";
				this->m_confirmationMessage += accountInfo.nickname;
				this->m_confirmationMessage += " [aid: ";
				this->m_confirmationMessage += std::to_string(accountInfo.accountID);
				this->m_confirmationMessage += "]\n";
				response.setOption(m_confirmationMessage.size());
				sendConfirmation(response, session);
			}
			this->m_confirmationMessage += "success"; // Additional arguments are ignored, e.g. /online foo
			sendConfirmation(response, session);
		}

		void OnlineCommand::getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response)
		{
			this->m_confirmationMessage += "/online: shows online players";
			sendConfirmation(response, session);
		}
	};
}
