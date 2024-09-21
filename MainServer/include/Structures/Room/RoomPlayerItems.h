#ifndef ROOM_PLAYER_ITEMS_H
#define ROOM_PLAYER_ITEMS_H

#include <cstdint>
#include <array>
#include "../AccountInfo/MainAccountUniqueId.h"

// This structure represents the (equipped) items of a single player in a given room

namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct RoomPlayerItems
		{
			std::array<std::uint32_t, 10> equippedItems{};
			std::array<std::uint32_t, 7> equippedWeapons{};
			Main::Structures::UniqueId uniqueId{};
		};
#pragma pack(pop)
	}
}


#endif