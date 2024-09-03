#ifndef MAIN_CHAT_COMMANDS_H
#define MAIN_CHAT_COMMANDS_H

#include <unordered_map>
#include <string>
#include <memory>
#include "ChatCommands/ISimpleCommand.h"
#include "ChatCommands/ISimpleRoomCommand.h"
#include "ChatCommands/ComplexCommands/IComplexCommand.h"
#include "ChatCommands/DatabaseCommands/IComplexDbCommand.h"
#include "../Network/MainSession.h"
#include "../Network/MainSessionManager.h"
#include "Network/Packet.h"
#include "../Classes/RoomsManager.h"
#include <ChatCommands/ComplexCommands/IComplexRoomCommand.h>


namespace Main
{
	namespace Command
	{
		class ChatCommands
		{
		public:
			using MainSimpleCommand =  Common::Command::ISimpleCommand<Main::Network::Session, Main::Network::SessionsManager>;
			using MainComplexCommand = Common::Command::IComplexCommand<Main::Network::Session, Main::Network::SessionsManager>;
			using MainSimpleRoomCommand = Common::Command::ISimpleRoomCommand<Main::Network::Session, Main::Classes::RoomsManager>;
			using MainComplexRoomCommand = Common::Command::IComplexRoomCommand<Main::Network::Session, Main::Classes::RoomsManager>;
			using DatabaseCommand = Common::Command::IComplexDbCommand<Main::Network::Session, Main::Persistence::PersistentDatabase>;

		private:
			std::unordered_map<std::string, std::unique_ptr<MainSimpleCommand>> m_simpleCommands;
			std::unordered_map<std::string, std::unique_ptr<MainSimpleRoomCommand>> m_simpleRoomCommands;
			std::unordered_map<std::string, std::unique_ptr<MainComplexCommand>> m_complexCommands;
			std::unordered_map<std::string, std::unique_ptr<DatabaseCommand>> m_databaseCommands;
			std::unordered_map<std::string, std::unique_ptr<MainComplexRoomCommand>> m_complexRoomCommands;

		public:
			ChatCommands() = default;

			void addSimpleCommand(const std::string& commandName, std::unique_ptr<MainSimpleCommand> mainSimpleCommand);

			void addComplexCommand(const std::string& commandName, std::unique_ptr<MainComplexCommand> mainComplexCommand);

			void addSimpleRoomCommand(const std::string& commandName, std::unique_ptr<MainSimpleRoomCommand> mainSimpleRoomCommand);

			void addDatabaseCommand(const std::string& commandName, std::unique_ptr<DatabaseCommand> databaseCommand);

			bool executeDatabaseCommand(const std::string& commandName, const std::string& wholeCommand, Main::Network::Session& session, 
				Main::Persistence::PersistentDatabase& database, Common::Network::Packet& response);

			bool executeSimpleCommand(const std::string& commandName, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager,
				Common::Network::Packet& response);

			bool executeComplexCommand(const std::string& commandName, const std::string& wholeCommand, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager,
				Common::Network::Packet& response);

			bool executeSimpleRoomCommand(const std::string& commandName, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager, std::size_t roomNumber,
				Common::Network::Packet& response);

			void addComplexRoomCommand(const std::string& commandName, std::unique_ptr<MainComplexRoomCommand> mainComplexCommand);

			bool executeComplexRoomCommand(const std::string& commandName, const std::string& wholeCommand, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager,
				Common::Network::Packet& response);

			void showUsages(Main::Network::Session& session, Common::Network::Packet& response, Common::Enums::PlayerGrade playerGrade);
		};
	}
}

#endif