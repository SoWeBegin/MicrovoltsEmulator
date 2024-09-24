
#ifndef MAIN_SERVER_H
#define MAIN_SERVER_H

#include <optional>
#include <asio.hpp>

#include "../include/Persistence/MainScheduler.h"
#include "Network/MainSessionManager.h"
#include "ChatCommands/ChatCommands.h"
#include "Classes/RoomsManager.h"

#include <iostream>

namespace Main
{
	using tcp = asio::ip::tcp;
	using ioContext = asio::io_context;

	class MainServer
	{
	private:
		ioContext& m_io_context;
		tcp::acceptor m_acceptor;
		std::optional<tcp::socket> m_socket;
		std::uint16_t m_serverId;
		Main::Persistence::PersistentDatabase m_database;
		Main::Persistence::MainScheduler m_scheduler;
		Main::Network::SessionsManager m_sessionsManager;
		Main::Classes::RoomsManager m_roomsManager;
		Main::Command::ChatCommands m_chatCommands;
		std::uint64_t m_timeSinceLastRestart{};

	public:
		std::vector<std::thread> threads;


	public:
		MainServer(ioContext& io_context, std::uint16_t port, std::uint16_t serverId);
		~MainServer() { for (std::thread &t : threads) t.join(); };
		void asyncAccept();
		void initializeAllCommands();
	};
}

#endif
