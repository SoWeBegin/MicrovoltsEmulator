

#include <string>
#include <charconv>
#include "../../../../include/Network/MainSession.h"
#include "../../../../include/Network/MainSessionManager.h"
#include "../../../../include/ChatCommands/ComplexCommands/OneArgumentCommands/SetLevel.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ComplexCommands/OneArgumentCommands/AbstractOneArgNumericalCommand.h"


namespace Main
{
	namespace Command
	{
		SetLevel::SetLevel(const Common::Enums::PlayerGrade requiredGrade)
			: AbstractOneArgNumericalCommand{ requiredGrade }
		{
		}

		void SetLevel::execute(const std::string& providedCommand, Main::Network::Session& session, Main::Network::SessionsManager& sessionManager, Common::Network::Packet& response)
		{
			if (!parseCommand(providedCommand))
			{
				this->m_confirmationMessage += "error: parsing error";
				sendConfirmation(response, session);
				return;
			}
			if (m_value < 0 || m_value > 105)
			{
				this->m_confirmationMessage += "error: wrong level";
				sendConfirmation(response, session);
				return;
			}

			this->m_confirmationMessage += "success (relog)";
			session.setLevel(m_value);
			sendConfirmation(response, session);

			/* N.B this cause issues, do NOT use
			response.setOrder(413);
			response.setExtra(1); // Check whether we should send 1, 54 or something else.
			response.setOption(0);
			response.setMission(0);
			session.setLevel(m_value);
			session.sendAccountInfo(response);
			*/
		}

		void SetLevel::getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response)
		{
			this->m_confirmationMessage += "/setlevel level: changes your level to level";
			sendConfirmation(response, session);
		}

	};
}
