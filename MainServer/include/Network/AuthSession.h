#ifndef MAIN_AUTH_SESSION_H
#define MAIN_AUTH_SESSION_H

#include <asio.hpp>
#include <iostream> // debug, remove later
#include "MainSessionManager.h"

// Used for Main<=>Auth IPC communication
// (AuthServer acts as a client here, retrieves certain info, and MainServer responds to its requests)
namespace Main 
{
    namespace Network 
    {
        class AuthSession : public std::enable_shared_from_this<AuthSession> 
        {
        private:
            asio::ip::tcp::socket m_socket;
            Main::Network::SessionsManager& m_sessionsManager;

        public:
            AuthSession(asio::ip::tcp::socket socket, Main::Network::SessionsManager& sessionsManager)
                : m_socket(std::move(socket)) 
                , m_sessionsManager{ sessionsManager }
            {
            }

            void start() 
            {
                auto self(shared_from_this());
                auto requestBuffer = std::make_shared<asio::streambuf>();
                asio::async_read_until(m_socket, *requestBuffer, "\n",
                    [this, self, requestBuffer](asio::error_code ec, std::size_t length) 
                    {
                        if (!ec) 
                        {
                            std::istream requestStream(requestBuffer.get());
                            std::string request;
                            std::getline(requestStream, request);

                            if (request == "get_player_count")
                            {
                                sendPlayerCount();
                            }
                            else if (request.find("is_player_online") == 0)
                            {
                                disconnectPlayerIfOnline(request.substr(17));
                            }
                        }
                    });
            }

        private:
            void sendPlayerCount() 
            {
                auto self(shared_from_this());
                const std::string totalPlayersOnline = std::to_string(m_sessionsManager.getTotalSessions()) + "\n";
                asio::async_write(m_socket, asio::buffer(totalPlayersOnline),
                    [this, self](asio::error_code ec, std::size_t length) 
                    {
                    });
            }

            void disconnectPlayerIfOnline(const std::string& accountIDStr)
            {
                auto self(shared_from_this());

                std::uint32_t accountID;
                auto [ptr, ec] = std::from_chars(accountIDStr.data(), accountIDStr.data() + accountIDStr.size(), accountID, 10);

                std::string response = "removed";
                if (ec != std::errc{} || ptr != accountIDStr.data() + accountIDStr.size())
                {
                    asio::async_write(m_socket, asio::buffer("Invalid account ID\n"),
                        [this, self](asio::error_code ec, std::size_t length)
                        {
                        });
                    response = "not_removed"; 
                }

                auto targetSession = m_sessionsManager.getSessionByAccountId(accountID);
                if (targetSession)
                {
                    Common::Network::Packet response;
                    response.setTcpHeader(targetSession->getId(), Common::Enums::USER_LARGE_ENCRYPTION);
                    m_sessionsManager.removeSession(targetSession->getId());
                    response.setOrder(73);
                    response.setExtra(5);
                    targetSession->asyncWrite(response);
                }

                asio::async_write(m_socket, asio::buffer(response),
                    [this, self](asio::error_code ec, std::size_t length)
                    {
                    });
            }
        };
    }
}

#endif