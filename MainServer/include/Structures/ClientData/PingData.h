#ifndef CLIENT_PING_DATA_MAIN_H
#define CLIENT_PING_DATA_MAIN_H

#include <cstdint>

namespace Main
{
	namespace ClientData
	{
#pragma pack(push, 1)
		struct Ping
		{
			std::uint32_t unused : 10 = 0; // recheck whether this really is unused
			std::uint32_t ping : 10 = 0;
			std::uint32_t rest : 12 = 0;
		};
#pragma pack(pop)
	}
}
#endif