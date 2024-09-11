
#include <string>
#include <memory>
#include "ChatCommands/ISimpleCommand.h"
#include "ChatCommands/ISimpleRoomCommand.h"
#include "ChatCommands/ComplexCommands/IComplexCommand.h"
#include "../../include/Network/MainSession.h"
#include "../../include/Network/MainSessionManager.h"
#include "Network/Packet.h"
#include "../../include/Classes/Room.h"
#include "../../include/Classes/RoomsManager.h"
#include "../../include/ChatCommands/ChatCommands.h"


namespace Main
{
	namespace Command
	{
		void ChatCommands::addSimpleCommand(const std::string& commandName, std::unique_ptr<MainSimpleCommand> mainSimpleCommand)
		{
			m_simpleCommands[commandName] = std::move(mainSimpleCommand);
		}

		void ChatCommands::addComplexCommand(const std::string& commandName, std::unique_ptr<MainComplexCommand> mainComplexCommand)
		{
			m_complexCommands[commandName] = std::move(mainComplexCommand);
		}

		void ChatCommands::addSimpleRoomCommand(const std::string& commandName, std::unique_ptr<MainSimpleRoomCommand> mainSimpleRoomCommand)
		{
			m_simpleRoomCommands[commandName] = std::move(mainSimpleRoomCommand);
		}

		void ChatCommands::addDatabaseCommand(const std::string& commandName, std::unique_ptr<DatabaseCommand> databaseCommand)
		{
			m_databaseCommands[commandName] = std::move(databaseCommand);
		}

		bool ChatCommands::executeSimpleCommand(const std::string& commandName, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager,
			Common::Network::Packet& response)
		{
			if (m_simpleCommands.contains(commandName))
			{
				const auto& accountInfo = session.getAccountInfo();
				if (accountInfo.playerGrade < m_simpleCommands[commandName]->getRequiredGrade()) return false;
				m_simpleCommands[commandName]->execute(session, sessionsManager, response);
				return true;
			}
			return false;
		}

		bool ChatCommands::executeComplexCommand(const std::string& commandName, const std::string& wholeCommand, Main::Network::Session& session,
			Main::Network::SessionsManager& sessionsManager, Common::Network::Packet& response)
		{
			if (m_complexCommands.contains(commandName))
			{
				const auto& accountInfo = session.getAccountInfo();
				if (accountInfo.playerGrade < m_complexCommands[commandName]->getRequiredGrade()) return false;
				m_complexCommands[commandName]->execute(wholeCommand, session, sessionsManager, response);
				return true;
			}
			return false;
		}

		bool ChatCommands::executeSimpleRoomCommand(const std::string& commandName, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager, std::size_t roomNumber,
			Common::Network::Packet& response)
		{
			if (m_simpleRoomCommands.contains(commandName))
			{
				const auto& accountInfo = session.getAccountInfo();
				if (accountInfo.playerGrade < m_simpleRoomCommands[commandName]->getRequiredGrade()) return false;
				m_simpleRoomCommands[commandName]->execute(session, roomsManager, roomNumber, response);
				return true;
			}
			return false;
		}

		bool ChatCommands::executeDatabaseCommand(const std::string& commandName, const std::string& wholeCommand, Main::Network::Session& session, Main::Persistence::PersistentDatabase& database,
			Common::Network::Packet& response)
		{
			if (m_databaseCommands.contains(commandName))
			{
				const auto& accountInfo = session.getAccountInfo();
				if (accountInfo.playerGrade < m_databaseCommands[commandName]->getRequiredGrade()) return false;
				m_databaseCommands[commandName]->execute(wholeCommand, session, database, response);
				return true;
			}
			return false;
		}

		void ChatCommands::addComplexRoomCommand(const std::string& commandName, std::unique_ptr<MainComplexRoomCommand> mainComplexCommand)
		{
			m_complexRoomCommands[commandName] = std::move(mainComplexCommand);
		}

		bool ChatCommands::executeComplexRoomCommand(const std::string& commandName, const std::string& wholeCommand, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager,
			Common::Network::Packet& response)
		{
			if (m_complexRoomCommands.contains(commandName))
			{
				const auto& accountInfo = session.getAccountInfo();
				if (accountInfo.playerGrade < m_complexRoomCommands[commandName]->getRequiredGrade()) return false;
				m_complexRoomCommands[commandName]->execute(wholeCommand, session, roomsManager, response);
				return true;
			}
			return false;
		}

		void ChatCommands::showUsages(Main::Network::Session& session, Common::Network::Packet& response, Common::Enums::PlayerGrade playerGrade)
		{
			for (const auto& currentSimple : m_simpleCommands)
			{
				if (playerGrade < currentSimple.second->getRequiredGrade()) continue;
				currentSimple.second->getCommandUsage(session, response);
			}
			for (const auto& currentComplex : m_complexCommands)
			{
				if (playerGrade < currentComplex.second->getRequiredGrade()) continue;
				if (currentComplex.first == "tip") continue; // shown already inside /announcement getUsage()
				currentComplex.second->getCommandUsage(session, response);
			}
			for (const auto& currentSimpleRoom : m_simpleRoomCommands)
			{
				if (playerGrade < currentSimpleRoom.second->getRequiredGrade()) continue;
				currentSimpleRoom.second->getCommandUsage(session, response);
			}
			for (const auto& currentDb : m_databaseCommands)
			{
				if (playerGrade < currentDb.second->getRequiredGrade()) continue;
				currentDb.second->getCommandUsage(session, response);
			}
			for (const auto& currentComplexRoom : m_complexRoomCommands)
			{
				if (playerGrade < currentComplexRoom.second->getRequiredGrade()) continue;
				currentComplexRoom.second->getCommandUsage(session, response);
			}
		}
	};
}

