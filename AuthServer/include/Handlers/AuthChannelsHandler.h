#ifndef AUTH_CHANNELS_HANDLER_H
#define AUTH_CHANNELS_HANDLER_H

#include <array>
#include <string>
#include <iostream>

#include "Network/Session.h"
#include "Network/Packet.h"
#include "../Structures/AuthChannels.h"
#include "../AuthEnums.h"
#include "../Constants.h"


namespace Auth
{
	namespace Handlers
	{
        inline std::uint32_t getPlayerCountFromMain()
        {
            asio::io_context ioContext;
            asio::ip::tcp::socket socket(ioContext);
            asio::ip::tcp::resolver resolver(ioContext);

            asio::connect(socket, resolver.resolve(Auth::Constants::mainServerIp, std::to_string(Auth::Constants::mainServerPort)));

            const std::string request = "get_player_count\n";
            asio::write(socket, asio::buffer(request));

            asio::streambuf responseBuffer;
            std::uint32_t playerCount = 0;

            try
            {
                asio::error_code ec;
                asio::read_until(socket, responseBuffer, "\n", ec);

                if (ec)
                {
                    asio::error_code ec;
                    socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
                    socket.close();
                    return 0; 
                }

                std::istream responseStream(&responseBuffer);
                std::string response;
                std::getline(responseStream, response);

                auto [ptr, ecc] = std::from_chars(response.data(), response.data() + response.size(), playerCount);
                if (ecc != std::errc{})
                {
                    asio::error_code ec;
                    socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
                    socket.close();
                    return 0;
                }

                return playerCount;
            }
            catch (const std::exception& e)
            {
                asio::error_code ec;
                socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
                socket.close();
                return 0;
            }

            asio::error_code ec;
            socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
            socket.close();
        }

		inline void handleServerChannelsInfo(const Common::Network::Packet& request, Common::Network::Session& session)
		{
            // Note: here, there should be totalPlayers for all available servers -- we assume we only have 1 main server, connected between different channels!
			std::uint32_t totalPlayers = getPlayerCountFromMain(); // actually use MainServer's IP here (obviously)

			Auth::Structures::ChannelsInfo channelsInfo{};
			using Status = Auth::Enums::ChannelStatus;
            Status channelStatus = Status::LOW_TRAFFIC;
            if (totalPlayers <= 20) channelStatus = Status::LOW_TRAFFIC;
            else if (totalPlayers <= 50) channelStatus = Status::MEDIUM_TRAFFIC;
            else if (totalPlayers <= 100) channelStatus = Status::HIGH_TRAFFIC;

			const std::array<std::uint32_t, 2> channels { Status::MEDIUM_TRAFFIC, Status::HIGH_TRAFFIC };
			channelsInfo.initializeChannels(channels);

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setCommand(23, 0, 0, static_cast<std::uint8_t>(channels.size()));
			response.setData(reinterpret_cast<std::uint8_t*>(&channelsInfo), sizeof(channelsInfo));
			session.asyncWrite(response);
		}
	}
}
#endif