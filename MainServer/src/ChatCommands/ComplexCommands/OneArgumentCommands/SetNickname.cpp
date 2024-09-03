

#include <string>
#include <charconv>
#include "../../../../include/Network/MainSession.h"
#include "../../../../include/Network/MainSessionManager.h"
#include "../../../../include/ChatCommands/ComplexCommands/OneArgumentCommands/SetNickname.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ComplexCommands/OneArgumentCommands/AbstractOneArgNonNumericalCommand.h"


namespace Main
{
	namespace Command
	{
		SetNickname::SetNickname(const Common::Enums::PlayerGrade requiredGrade)
			: AbstractOneArgNonNumericalCommand<Main::Network::Session, Main::Network::SessionsManager>{ requiredGrade }
		{
		}

		void SetNickname::execute(const std::string& providedCommand, Main::Network::Session& session, Main::Network::SessionsManager& sessionManager, Common::Network::Packet& response)
		{
			if (!parseCommand(providedCommand))
			{
				this->m_confirmationMessage += "error: parsing error";
				sendConfirmation(response, session);
				return;
			}
			if (m_value.size() >= 16)
			{
				this->m_confirmationMessage += "error: nickname must contain less than 16 characters";
				sendConfirmation(response, session);
				return;
			}

			session.setPlayerName(m_value.c_str());
			this->m_confirmationMessage += "success (relog)";
			sendConfirmation(response, session);

			/* N.B do NOT use, this causes bugs:
			response.setOrder(413);
			response.setExtra(1); // Check whether we should send 1, 54 or something else.
			response.setOption(0);
			response.setMission(0);
			session.setPlayerName(m_value.c_str());
			session.sendAccountInfo(response);
			*/
		}

		void SetNickname::getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response)
		{
			this->m_confirmationMessage += "/setname name: changes name to the specified one";
			sendConfirmation(response, session);
		}

	};
}
