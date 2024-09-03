
#ifndef CAST_SERVER_H
#define CAST_SERVER_H

#include <cstdint>
#include <asio.hpp>
#include <optional>
#include "Classes/RoomsManager.h"
#include "Network/SessionsManager.h"

namespace Cast
{
	using tcp = asio::ip::tcp;
	using ioContext = asio::io_context;

	class CastServer
	{
	private:
		ioContext& m_io_context;
		tcp::acceptor m_acceptor;
		std::optional<tcp::socket> m_socket;
		std::uint16_t m_serverId;
		Cast::Network::SessionsManager m_sessionsManager{};
		Cast::Classes::RoomsManager m_roomsManager{};

	public:
		CastServer(ioContext& io_context, std::uint16_t port, std::uint16_t serverId);
		void asyncAccept();
	};
}

#endif
