#ifndef ROOMS_LIST_STRUCTURE_H
#define ROOMS_LIST_STRUCTURE_H

#include <cstdint>
#include <vector>

namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct SingleRoom
		{
			char title[30]{};
			std::uint16_t padding{};
			std::uint16_t roomNumber{};
			std::uint16_t roomId : 9 = 2; // if this is 0 you can't join the room...?!  // THIS ONE IS SENT FROM CLIENT=>SERVER WHEN YOU JOIN THE ROOM, ALONG WITH ROOM NUM!!!
			std::uint16_t map : 7 = 0;
			std::uint32_t mode : 5 = 0; // if 15 = room not displayed, if > 15 == clan related, else if < 15 == mode, else if == 14 then more infos for aibattle
			std::uint32_t maxPlayers : 5 = 0;
			std::uint32_t numPlayers : 5 = 0;
			std::uint32_t matchStarted : 1 = 0;
			std::uint32_t hasPassword : 1 = 0;
			std::uint32_t isObserverOff : 1 = 0;
			std::uint32_t weaponRestriction : 4 = 0;
			std::uint32_t unknown1 : 1 = 1; // keep this as 1, otherwise observer on/off doesn't work properly
			std::uint32_t ping : 9 = 0;
		};
#pragma pack(pop)

#pragma pack(push, 1)
		struct RoomsList
		{
			std::uint16_t totalRooms1{};
			std::uint16_t totalRooms2{}; // apparently must be the same as totalRooms1
			std::vector<SingleRoom> rooms{};
		};
#pragma pack(pop)
	}
}
#endif