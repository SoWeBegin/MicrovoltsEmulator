#ifndef AUTH_CONSTANTS_H
#define AUTH_CONSTANTS_H

#include <string>

namespace Auth
{
	namespace Constants
	{
		constexpr inline std::string mainServerIp = "127.0.0.1"; // change this if you use a different main server IP address
		constexpr inline std::uint16_t mainServerPort = 13004; // keep this as it is
	}
}

#endif