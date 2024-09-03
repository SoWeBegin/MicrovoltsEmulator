
#include <string>
#include <charconv>
#include "../../../include/Network/MainSession.h"
#include "../../../include/Network/MainSessionManager.h"
#include "../../../include/ChatCommands/DatabaseCommands/UnbanOfflinePlayer.h"
#include "Enums/PlayerEnums.h"
#include "ChatCommands/DatabaseCommands//AbstractOneArgNonNumericalDbCommand.h"

namespace Main
{
	namespace Command
	{

		UnbanAccount::UnbanAccount(const Common::Enums::PlayerGrade requiredGrade)
			: AbstractOneArgNonNumericalDbCommand<Main::Network::Session, Main::Persistence::PersistentDatabase>{ requiredGrade }
		{
		}

		void UnbanAccount::execute(const std::string& providedCommand, Main::Network::Session& session, Main::Persistence::PersistentDatabase& database,
			Common::Network::Packet& response) 
		{
			if (!parseCommand(providedCommand))
			{
				this->m_confirmationMessage += "error: parsing error";
				sendConfirmation(response, session);
				return;
			}

			const bool unbanned = database.unbanPlayer(m_value);
			if (!unbanned)
			{
				this->m_confirmationMessage += "error";
			}
			else
			{
				this->m_confirmationMessage += "success";
			}
			sendConfirmation(response, session);
		}

		void UnbanAccount::getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response) 
		{
			this->m_confirmationMessage += "/unban nickname: unbans a player identified by nickname that was previously banned";
			sendConfirmation(response, session);
		}

	};
}
