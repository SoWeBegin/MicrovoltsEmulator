

#include <string>
#include <charconv>
#include "../../../../include/Network/MainSession.h"
#include "../../../../include/Network/MainSessionManager.h"
#include "../../../../include/ChatCommands/ComplexCommands/OneArgumentCommands/CreateAnnouncement.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/ComplexCommands/OneArgumentCommands/AbstractOneArgNonNumericalCommand.h"


namespace Main
{
	namespace Command
	{
		CreateAnnouncement::CreateAnnouncement(const Common::Enums::PlayerGrade requiredGrade)
			: AbstractOneArgNonNumericalCommand<Main::Network::Session, Main::Network::SessionsManager>{ requiredGrade }
		{
		}

		void CreateAnnouncement::execute(const std::string& providedCommand, Main::Network::Session& session, Main::Network::SessionsManager& sessionManager, Common::Network::Packet& response)
		{
			std::string str = "announce";
			std::string str2 = "tip";
			std::string tipOrAnnounce;
			auto cmd = providedCommand;
			if (cmd.find(str) == 0)
			{
				tipOrAnnounce = str;
				cmd.erase(0, str.length() + 1);
			}
			else if (cmd.find(str2) == 0)
			{
				tipOrAnnounce = str2;
				cmd.erase(0, str2.length() + 1);
			}
			else
			{
				this->m_confirmationMessage += "error";
				sendConfirmation(response, session);
				return;
			}

			if (m_value.size() > 512)
			{
				this->m_confirmationMessage += "error: the message must be smaller than 512 characters";
				sendConfirmation(response, session);
				return;
			}

			this->m_confirmationMessage += "success";
			sendConfirmation(response, session);

			response.setOrder(402);
			response.setExtra(tipOrAnnounce == "announce" ? 0xA : 0xC);
			response.setData(reinterpret_cast<std::uint8_t*>(const_cast<char*>(cmd.c_str())), cmd.size());
			sessionManager.broadcast(response);
		}

		void CreateAnnouncement::getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response)
		{
			this->m_confirmationMessage += "/announce message: sends an announcement with the given message (max. 512 characters)";
			sendConfirmation(response, session);
			m_confirmationMessage = std::string(16, '0');
			this->m_confirmationMessage += "/tip message: sends a tip with the given message (max. 512 characters)";
			sendConfirmation(response, session);
		}

	};
}
