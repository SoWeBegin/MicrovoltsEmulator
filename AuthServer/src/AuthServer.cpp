#include "../include/AuthServer.h"
#include "../include/Network/AuthSession.h"
#include "../include/Handlers/AuthAllHandlers.h"
#include "../include/Structures/AuthConnection.h"


namespace Auth
{
	AuthServer::AuthServer(ioContext& io_context, const std::string & path, std::uint16_t port = 13000)
		: m_io_context(io_context)
		, m_acceptor{ io_context, tcp::endpoint(asio::ip::address::from_string("0.0.0.0"), port) }
		//, m_database("../ExternalLibraries/Database/GameDatabase.db")
		, m_database(path)
	{
		Common::Network::Session::addCallback<Auth::Network::Session>(22, [&](const Common::Network::Packet& request,
			Auth::Network::Session& session) { Auth::Handlers::handleAuthUserInformation(request, session, m_database); });

		Common::Network::Session::addCallback<Auth::Network::Session>(23, Auth::Handlers::handleServerChannelsInfo);

		Common::Network::Session::addCallback<Auth::Network::Session>(25, Auth::Handlers::handleAccountBusyReconnection);
	}

	void AuthServer::asyncAccept()
	{
		m_socket.emplace(m_io_context);
		m_acceptor.async_accept(*m_socket, [&](asio::error_code error)
			{
				using PlayerMgr = Auth::Player::AuthPlayerManager;
				using ServerType = Common::Enums::ServerType;

				auto client = std::make_shared<Auth::Network::Session>(std::move(*AuthServer::m_socket),
					std::bind(&PlayerMgr::remove, &PlayerMgr::getInstance(), std::placeholders::_1));

				client->sendConnectionACK(ServerType::AUTH_SERVER);
				asyncAccept();
			});
	}
}
