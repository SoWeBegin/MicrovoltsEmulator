#ifndef MAIN_ACCOUNT_MUTEINFO_H
#define MAIN_ACCOUNT_MUTEINFO_H

#include <cstdint>
#include <string>

namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct MuteInfo
		{
			bool isMuted;
			std::string reason;
			std::string mutedBy;
			std::string mutedUntil;
		};
#pragma pack(pop)
	}
}

#endif