#ifndef ROOM_PLAYER_CLAN_H
#define ROOM_PLAYER_CLAN_H

#include <cstdint>

namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct PlayerClan
		{
			char clanName[16]{};
			std::uint64_t unknown : 4 = 0; // 0 = clan displayed,  1 or 14 = no clan displayed (test other values)
			std::uint64_t clanLogoFrontId : 16 = 0;
			std::uint64_t clanLogoBackId : 14 = 0;
			std::uint64_t unknown2 : 27 = 0; // maybe clan id?
			std::uint64_t unused : 3 = 0;
		};
#pragma pack(pop)
	}
}
#endif