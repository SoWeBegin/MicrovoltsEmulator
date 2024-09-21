
#include <string>
#include <charconv>
#include "../../../include/Network/MainSession.h"
#include "../../../include/Network/MainSessionManager.h"
#include "../../../include/ChatCommands/DatabaseCommands/AddPlayer.h"
#include "Enums/PlayerEnums.h"
#include "../../../Common/include/Utils/Utils.h"

namespace Main
{
	namespace Command
	{

		AddPlayer::AddPlayer(const Common::Enums::PlayerGrade requiredGrade)
			: AbstractThreeArgCommandDb<Main::Network::Session, Main::Persistence::PersistentDatabase>{ requiredGrade }
		{
		}

		void AddPlayer::execute(const std::string& providedCommand, Main::Network::Session& session, Main::Persistence::PersistentDatabase& database,
			Common::Network::Packet& response)
		{
			if (!parseCommand(providedCommand))
			{
				this->m_confirmationMessage += "error: parsing error";
				sendConfirmation(response, session);
				return;
			}
			if (m_nickname.size() >= 15)
			{
				this->m_confirmationMessage += "error: max. player nickname size is 16";
				sendConfirmation(response, session);
				return;
			}

			const bool added = database.addPlayer(m_username, Common::Utils::calculateHashCryptoPP<CryptoPP::SHA256>(m_password), m_nickname);
			if (!added)
			{
				this->m_confirmationMessage += "error";
			}
			else
			{
				this->m_confirmationMessage += "success";
			}
			sendConfirmation(response, session);
		}

		void AddPlayer::getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response)
		{
			this->m_confirmationMessage += "/addplayer username password nickname: adds a new player";
			sendConfirmation(response, session);
		}

	};
}
