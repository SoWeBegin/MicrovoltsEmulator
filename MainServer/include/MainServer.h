
#ifndef MAIN_SERVER_H
#define MAIN_SERVER_H

#include <optional>
#include <asio.hpp>

#include "../include/Persistence/MainScheduler.h"
#include "Network/MainSessionManager.h"
#include "ChatCommands/ChatCommands.h"
#include "Classes/RoomsManager.h"
#include "Classes/CapsuleManager.h"

#include <iostream>
#include "Boxes/BoxBase.h"

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
		Main::Classes::CapsuleManager m_capsuleManager;
		Main::Network::SessionsManager m_sessionsManager;
		Main::Classes::RoomsManager m_roomsManager;
		Main::Command::ChatCommands m_chatCommands;
		std::unordered_map<std::uint32_t, std::unique_ptr<Main::Box::IBox>> m_boxes;
		std::uint64_t m_timeSinceLastRestart{};


		// For auth server communication
		tcp::acceptor m_authServerAcceptor;
		std::optional<tcp::socket> m_authSocket; 


	public:
		std::vector<std::jthread> threads;

	public:
		MainServer(ioContext& io_context, std::uint16_t clientPort, std::uint16_t authPort, std::uint16_t serverId);
		void asyncAccept();
		void asyncAcceptAuthServer();
		void initializeAllCommands();
		void initializeBoxes();
	};
}

#endif
