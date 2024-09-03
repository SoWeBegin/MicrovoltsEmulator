#ifndef AUTH_SESSION_HEADER
#define AUTH_SESSION_HEADER

#include <functional>
#include <chrono>
#include <iostream>

#include <asio.hpp>
#include "Network/Session.h"

namespace Auth
{
	namespace Network
	{
		struct Session : public Common::Network::Session
		{
			using tcp = asio::ip::tcp;

			explicit Session(tcp::socket&& socket, std::function<void(std::size_t)> fnct);

			void onPacket(std::vector<std::uint8_t>& data) override;
		};
	}
}

#endif