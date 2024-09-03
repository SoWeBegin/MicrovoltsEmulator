#ifndef AUTH_SERVERCHANNELINFO_H
#define AUTH_SERVERCHANNELINFO_H

#include <array>

#include "Protocol/CommandHeader.h"
#include "Protocol/TcpHeader.h"
#include "Utils/RandomGeneration.h"
#include "Enums/MiscellaneousEnums.h"


namespace Auth
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct ChannelsInfo
		{
			std::array<std::uint32_t, 6> channels{};

			void initializeChannels(std::array<std::uint32_t, 6> givenChannels)
			{
				for (std::size_t idx = 0; std::uint32_t & current : channels)
				{
					current = static_cast<std::uint32_t>((idx + 1) | givenChannels[idx++]);
				}
			}
		};
#pragma pack(pop)
	}
}

#endif