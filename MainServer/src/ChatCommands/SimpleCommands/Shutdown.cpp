
#include <string>
#include <charconv>
#include "../../../include/Network/MainSession.h"
#include "../../../include/Network/MainSessionManager.h"
#include "../../../include/ChatCommands/SimpleCommands/Shutdown.h"
#include "Enums/PlayerEnums.h"

#include "ChatCommands/ISimpleCommand.h"

namespace Main
{
	namespace Command
	{
		// Todo: Execution of this command unequips all equipped items, fix that.
		Shutdown::Shutdown(const Common::Enums::PlayerGrade requiredGrade)
			: ISimpleCommand{ requiredGrade }
		{
		}

		void Shutdown::execute(Main::Network::Session& session, Main::Network::SessionsManager& sessionManager, Common::Network::Packet& response)
		{
			// Disconnect all the players
			for (auto& currentSession: sessionManager.getAllSessions())
			{
				currentSession.second->closeSocket();
			}

			std::terminate();
		}

		void Shutdown::getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response)
		{
			this->m_confirmationMessage += "/shutdown: closes the server (create an announcement before doing that)";
			sendConfirmation(response, session);
		}
	};
}
