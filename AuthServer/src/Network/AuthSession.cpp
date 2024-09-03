#include <functional>
#include <chrono>
#include <asio.hpp>
#include <iostream>

#include "../../include/Network/AuthSession.h"
#include "Network/Session.h"

namespace Auth
{
	namespace Network
	{
		Session::Session(tcp::socket&& socket, std::function<void(std::size_t)> fnct)
			: Common::Network::Session{ std::move(socket), fnct }
		{
		}

		void Session::onPacket(std::vector<std::uint8_t>& data)
		{
			std::optional<std::uint32_t> key = std::nullopt;
			if (m_crypt.isUsed) key = m_crypt.UserKey;

			Common::Network::Packet incomingPacket;
			incomingPacket.processIncomingPacket(data.data(), static_cast<std::uint16_t>(data.size()), key);

			const std::uint16_t callbackNum = incomingPacket.getOrder();
			if (!Common::Network::Session::callbacks<Session>.contains(callbackNum))
			{
				std::cerr << "[AuthSession] No callback for order: " << callbackNum << "\n";
				return;
			}

			Common::Network::Session::callbacks<Session>[callbackNum](incomingPacket, *this);
		}
	};
}
