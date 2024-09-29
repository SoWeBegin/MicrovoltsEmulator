#include <vector>
#include <iostream>
#include <cstring>
#include <ctime>
#include <optional>

#ifdef WIN32
#include <corecrt.h>
#else
#define __time32_t uint32_t
#endif

#include "../../include/Network/Session.h"
#include "../../include/Utils/Parser.h"
#include "../../include/Enums/ExtrasEnums.h"
#include "../../../MainServer/include/Structures/AccountInfo/MainAccountUniqueId.h"

namespace Common
{
	namespace Network
	{
		void Session::asyncWrite(const Common::Network::Packet& message)
		{
			std::unique_lock lock(m_sendMutex);
			std::optional<std::uint32_t> key = m_crypt.isUsed ? std::make_optional(m_crypt.UserKey) : std::nullopt;
			std::vector<uint8_t> encryptedMessage = message.generateOutgoingPacket(key);
			m_sendQueue.push(encryptedMessage);
			lock.unlock();

			bool expected = false, desired = true;
			if (m_isInSend.compare_exchange_strong(expected, desired))
			{
				write();
			}
			/*asio::async_write(m_socket, asio::buffer(encryptedMessage.data(), message.getFullSize()),
				[&, self = this->shared_from_this()](const asio::error_code& errorCode, std::size_t)
				{
					if (!errorCode)
					{
						//std::printf("Server message sent!\n");
					}
					else
					{
						std::printf("Failed to send server message: %s\n", errorCode.message().c_str());
					}
				});
				*/
		}

		void Session::write()
		{
			std::unique_lock lock(m_sendMutex);
			if (m_sendQueue.empty())
			{
				m_isInSend = false;
				return;
			}
			if (!m_socket.is_open())
			{
				return;
			}
			auto& nextMessage = m_sendQueue.front();
			lock.unlock();

			asio::async_write(m_socket, asio::buffer(nextMessage.data(), nextMessage.size()),
				[&, self = this->shared_from_this()](const asio::error_code& errorCode, std::size_t)
				{
					if (!errorCode)
					{
						std::unique_lock lock(m_sendMutex);
						self->m_sendQueue.pop();
						lock.unlock();
						self->write();
					}
					else
					{
						self->closeSocket();
						std::printf("Failed to send server message: %s\n", errorCode.message().c_str());
					}
				});
		}

		void Session::asyncRead()
		{
			if (!m_socket.is_open())
			{
				std::printf("Socket is not open");
				return;
			}

			m_socket.async_read_some(asio::buffer(m_buffer.data(), m_buffer.size()),
				[self = this->shared_from_this(), this](const asio::error_code& error, std::size_t bytes_transferred)
				{
					self->onRead(error, bytes_transferred);
					asyncRead();
				});
		}

		void Session::onRead(asio::error_code error, std::size_t bytes_transferred)
		{
			if (!error)
			{
				const constexpr int headerSize = sizeof(Common::Protocol::TcpHeader);
				m_reader.insert(m_reader.end(), m_buffer.begin(), m_buffer.begin() + bytes_transferred);

				//if (!m_crypt.isUsed) Common::Parser::parse_cast(m_reader.data(), m_reader.size(), 13000, "client", "server");
				//if (m_crypt.isUsed) Common::Parser::parse(m_reader.data(), m_reader.size(), 13000, "client", "server", m_crypt.UserKey);

				Common::Protocol::TcpHeader header;
				Common::Cryptography::Crypt cryptography(0);
				while (m_reader.size() > headerSize)
				{
					if (m_crypt.isUsed)
					{
						cryptography.RC5Decrypt32(reinterpret_cast<int32_t*>(m_reader.data()), &header, headerSize);
					}
					else
					{
#ifdef WIN32
						memcpy_s(&header, headerSize, m_reader.data(), headerSize);
#else
						memcpy(&header, m_reader.data(), headerSize);
#endif
					}

					if (header.getSize() >= 2047)
					{
						std::printf("Session::onRead() - Invalid packet size: %d\n", header.getSize());
						closeSocket();
						return;
					}

					if (m_reader.size() >= static_cast<std::size_t>(header.getSize()))
					{
						std::vector<std::uint8_t> data(m_reader.begin(), m_reader.begin() + header.getSize());
						onPacket(data);

						auto newSize = m_reader.size() - header.getSize();
						std::memmove(m_reader.data(), m_reader.data() + header.getSize(), newSize);
						m_reader.resize(newSize);
					}
					else
					{
						// avoid infinite loop
						break;
					}
				}
			}
			else
			{
				std::printf("onRead() Error: %s\n", error.message().c_str());
				closeSocket();
			}
		}


		std::uint8_t* Session::encryptRawMessage(std::uint8_t* data, std::size_t size)
		{
			constexpr std::size_t headerSize = sizeof(Common::Protocol::TcpHeader);
			m_defaultCrypt.RC5Encrypt64(data, data, headerSize);
			m_crypt.RC6Encrypt128(data + headerSize, data + headerSize, static_cast<int>(size - headerSize));
			return data;
		}

		void Session::closeSocket()
		{
			std::cout << "m_id: " << m_id << '\n';
			m_onCloseSocketCallback(m_id);
			if (!m_socket.is_open())
			{
				return;
			}
			asio::error_code errorCode;
			auto endPoint = m_socket.remote_endpoint(errorCode);
			std::printf("Session::closeSocket() - Connection closed from %s:%d\n", endPoint.address().to_string().c_str(), endPoint.port());
			m_socket.shutdown(tcp::socket::shutdown_both, errorCode);
			m_socket.close(errorCode);
		}

		std::uint8_t* Session::getBufferData()
		{
			return m_buffer.data();
		}

		std::size_t Session::getBufferSize() const
		{
			return m_buffer.size();
		}
		
		Common::Cryptography::Crypt Session::getUserCrypt() const
		{
			return m_crypt;
		}
		
		Common::Cryptography::Crypt Session::getDefaultCrypt() const
		{
			return m_defaultCrypt;
		}
		
		void Session::sendConnectionACK(Common::Enums::ServerType serverType)
		{
			Packet connectionAck;
			connectionAck.setTcpHeader(m_id, Common::Enums::EncryptionType::NO_ENCRYPTION);

			switch (serverType)
			{
				case Enums::AUTH_SERVER:
				{
					struct AuthAck
					{
						std::int32_t key{ static_cast<std::int32_t>(rand() + 1) };
						__time32_t timestamp{ static_cast<__time32_t>(std::time(0)) };
					} authAck;

					m_crypt.KeySetup(authAck.key);
					connectionAck.setData(reinterpret_cast<std::uint8_t*>(&authAck), sizeof(AuthAck));
					connectionAck.setCommand(401, 0, static_cast<int>(Common::Enums::AUTH_SUCCESS), 0);
					asyncWrite(connectionAck);
					break;
				}

				case Enums::MAIN_SERVER:
				{
					// Copy pasted, can't #include Main here due to circular dependencies
					struct UniqueId
					{
						std::uint32_t session : 16 = 0;
						std::uint32_t server : 15 = 4;
						std::uint32_t unknown : 1 = 0;
					};

					struct MainAck
					{
						std::int32_t key{ static_cast<std::int32_t>(rand() + 1) };
						UniqueId uniqueId{};
					} mainAck;

					// This allows the ping to work for both cast and main server.
					// This also sets the SessionID that the client will now use for this session's communication
					mainAck.uniqueId.session = m_id;
					mainAck.uniqueId.server = 4;

					m_crypt.KeySetup(mainAck.key);
					connectionAck.setData(reinterpret_cast<std::uint8_t*>(&mainAck), sizeof(MainAck));
					connectionAck.setCommand(401, 0, static_cast<int>(Common::Enums::MAIN_SUCCESS), 1);
					// nb. option = channel selected by user
					// nb. success = e.g. MAIN_SUCCESS
					asyncWrite(connectionAck);
					break;
				}

				case Enums::CAST_SERVER:
				{
					m_crypt.isUsed = false;
					struct CastAck
					{
						std::int32_t key{ static_cast<std::int32_t>(rand() + 1) };
					} castAck;
					connectionAck.setData(reinterpret_cast<std::uint8_t*>(&castAck), sizeof(castAck));
					connectionAck.setCommand(401, 0, Common::Enums::CAST_SUCCESS, 0);
					asyncWrite(connectionAck);
					break;
				}
			}

			asyncRead();
		}

		std::size_t Session::getId() const
		{
			return m_id;
		}
	}
}