#ifndef AUTH_CHANNELS_HANDLER_H
#define AUTH_CHANNELS_HANDLER_H


#include <array>

#include "Network/Session.h"
#include "Network/Packet.h"
#include "../Structures/AuthChannels.h"
#include "../AuthEnums.h"


namespace Auth
{
	namespace Handlers
	{
		inline void handleServerChannelsInfo(const Common::Network::Packet& request, Common::Network::Session& session)
		{
			Auth::Structures::ChannelsInfo channelsInfo{};
			using Status = Auth::Enums::ChannelStatus;
			const std::array<std::uint32_t, 6> channels {
				Status::OFFLINE, Status::OFFLINE, Status::OFFLINE, Status::LOW_TRAFFIC, Status::OFFLINE, Status::OFFLINE
			};
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