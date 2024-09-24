#ifndef CLIENT_ROOM_CREATION_INFO_H
#define CLIENT_ROOM_CREATION_INFO_H

#include <cstdint>
#include <iostream> 
// These structures are sent from client => server when a new room is created

// Team balance is apparently NOT sent ==> on room creation, set it up based on the chosen mode

namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct RoomSettings
		{
			std::uint32_t time : 5 = 0;
			std::uint32_t weaponRestriction : 4 = 0;
			std::uint32_t isItemOn : 1 = 0;
			std::uint32_t mode : 4 = 0;
			std::uint32_t unknown2 : 1 = 1;
			std::uint32_t isOpen : 1 = 0;
			std::uint32_t hasPassword : 1 = 0;
			std::uint32_t unknown1 : 1 = 0;
			std::uint32_t playersPerTeam : 4 = 0;
			std::uint32_t map : 7 = 0;
			std::uint32_t probablyTeam : 2 = 0;
			std::uint32_t isObserverModeOn : 1 = 0;

			void print() const
			{
				std::cout << "Time: " << time << std::endl;
				std::cout << "WeaponRestriction: " << weaponRestriction << std::endl;
				std::cout << "isItemOn: " << isItemOn << std::endl;
				std::cout << "mode: " << mode << std::endl;
				std::cout << "unknown2: " << unknown2 << std::endl;
				std::cout << "isOpen: " << isOpen << std::endl;
				std::cout << "hasPassword: " << hasPassword << std::endl;
				std::cout << "unknown1: " << unknown1 << std::endl;
				std::cout << "playersPerTeam: " << playersPerTeam << std::endl;
				std::cout << "map: " << map << std::endl;
				std::cout << "probablyTeam: " << probablyTeam << std::endl;
				std::cout << "isObserverModeOn: " << isObserverModeOn << std::endl;

			}
		};
#pragma pack(pop)

#pragma pack(push, 1)
		struct CompleteRoomInfo
		{
			RoomSettings roomSettings{};
			char title[30]{};
			std::uint16_t padding = 0;
			char password[8]{};
			std::uint64_t padding2 = 0; // password max length is 8 client side, no need to have 8 more useless characters
		};
#pragma pack(pop)
	}
}

#endif

