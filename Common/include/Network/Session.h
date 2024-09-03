#ifndef SESSION_HEADER
#define SESSION_HEADER

#include <asio.hpp>
#include <array>
#include <vector>
#include <unordered_map>
#include <functional>
#include <utility>
#include <type_traits>

#include "../Cryptography/Crypt.h"
#include "../Enums/MiscellaneousEnums.h"

#include "Packet.h"
#include "SessionIdManager.h"


namespace Common
{
	namespace Network
	{
		using tcp = asio::ip::tcp;

		class Session : public std::enable_shared_from_this<Session>
		{
		protected:
			tcp::socket m_socket;
			std::array<std::uint8_t, 1024> m_buffer{};
			std::vector<std::uint8_t> m_reader{};
			Common::Cryptography::Crypt m_crypt{};
			Common::Cryptography::Crypt m_defaultCrypt{};
			std::function<void(std::size_t)> m_onCloseSocketCallback{};
			std::size_t m_id = 0;

			template<class T>
			inline static std::unordered_map<std::uint16_t, std::function<void(const Packet&, T&)>> callbacks;
			inline static std::size_t id_counter = 1;
			inline static SessionIdManager sessionIdManager{ 500 };

		public:
			Session() = default;

			explicit Session(tcp::socket&& socket, std::function<void(std::size_t)> fnct)
				: m_socket{ std::move(socket) }
				, m_onCloseSocketCallback{ fnct }
			{
				auto newID = sessionIdManager.getNewSessionID();
				if (newID.has_value())
				{
					m_id = newID.value();
				}
				else
				{
					throw std::runtime_error("No available session IDs.");
				}
			}

			virtual ~Session()
			{
				sessionIdManager.releaseSessionID(m_id);
			}

			void setSessionId(std::size_t id)
			{
				m_id = id;
			}

			void asyncWrite(const Common::Network::Packet& message);
			void asyncRead();
			void onRead(asio::error_code error, std::size_t bytes_transferred);
			void closeSocket();
			virtual void onPacket(std::vector<std::uint8_t>& data) = 0;
			std::uint8_t* encryptRawMessage(std::uint8_t* data, std::size_t size);
			std::uint8_t* getBufferData();
			std::size_t getBufferSize() const;
			Common::Cryptography::Crypt getUserCrypt() const;
			Common::Cryptography::Crypt getDefaultCrypt() const;
			std::size_t getId() const;

			void sendConnectionACK(Common::Enums::ServerType serverType);

			
			template<typename T>
			static inline void addCallback(std::uint16_t idx, auto fnct)
			{
				callbacks<T>[idx] = fnct;
			}
		};
	}
}

#endif