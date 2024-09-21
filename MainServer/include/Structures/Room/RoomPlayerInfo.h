#ifndef ROOM_PLAYER_INFO_H
#define ROOM_PLAYER_INFO_H

#include <cstdint>
#include "../AccountInfo/MainAccountUniqueId.h"
#include <array>

// This structure represents a single player in a given room

namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)

		// Used to retrieve the infos or the players that are already in the room, then sent to the player that joined that room
		struct RoomPlayerInfo
		{
			Main::Structures::UniqueId uniqueId{};  // ok
			std::uint32_t unknown_0 : 4 = 0; // OK
			std::uint32_t unknown_1 : 3 = 0; // OK
			std::uint32_t character : 4 = 0;
			std::uint32_t team : 4 = 0;  // red=1, blue=2, obs=4
			std::uint32_t level : 7 = 0; // actual level is givenValue - 1!
			std::uint32_t unknown_2 : 10 = 0;
			char playerName[16]{};
			std::uint32_t unknown_3 : 2 = 0; 
			std::uint32_t state : 4 = 0; // e.g. 7=capsule (see related enum)
			std::uint32_t ping : 10 = 0;
			std::uint32_t unknown_5 : 16 = 0;
		};
#pragma pack(pop)

#pragma pack(push, 1)
		// Used to send the info of the player who joined to all the players that are already in the room
		struct RoomLatestEnteredPlayerInfo
		{
			Main::Structures::UniqueId uniqueId{};  // ok
			std::uint32_t unknown_0 : 4 = 0; // OK
			std::uint32_t unknown_1 : 3 = 0; // OK
			std::uint32_t character : 4 = 0;
			std::uint32_t team : 4 = 0;  // red=1, blue=2, obs=4
			std::uint32_t level : 7 = 0; // actual level is givenValue - 1!
			std::uint32_t unknown_2 : 10 = 0;
			std::array<std::uint32_t, 10> equippedItems{};
			std::array<std::uint32_t, 7> equippedWeapons{};
			char playerName[16]{};
			std::uint16_t unknown_3 : 3 = 0;
			std::uint16_t ping : 10 = 0; 
			std::uint16_t unknown_4 : 2 = 0;
			std::uint16_t padding : 1 = 0;
		};
#pragma pack(pop)
	}
}
#endif