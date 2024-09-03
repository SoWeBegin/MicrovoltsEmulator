#ifndef AUTHENTICATION_SERVER_H
#define AUTHENTICATION_SERVER_H


#include <optional>
#include <memory>

#include <asio.hpp>

#include "Player/AuthPlayerManager.h"
#include "Database/DbPlayerInfo.h"
#include "Enums/MiscellaneousEnums.h"

namespace Auth
{
	using tcp = asio::ip::tcp;
	using ioContext = asio::io_context;

	class AuthServer
	{
	private:
		ioContext& m_io_context;
		tcp::acceptor m_acceptor;
		std::optional<tcp::socket> m_socket;
		Auth::Persistence::PersistentDatabase m_database;

	public:
		AuthServer(ioContext& io_context, std::uint16_t port);
		void asyncAccept();
	};
}

#endif