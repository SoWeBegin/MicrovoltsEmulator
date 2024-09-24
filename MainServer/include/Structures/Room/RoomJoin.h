#ifndef ROOM_JOIN_STRUCTURE_H
#define ROOM_JOIN_STRUCTURE_H

#include <cstdint>

// This structure is sent from server => client when a room is entered

// MISSING: Can't find "Item ON/OFF" setting anywhere ==> was it implemented in this client version at all?
// MISSING: specific room setting [eli => num of rounds, TDM => num of total kills, etc.] 

namespace Main
{
	namespace Structures
	{
#pragma pack(push,1)
		struct RoomJoin
		{
			std::uint32_t map : 7 = 0;
			std::uint32_t mode : 5 = 0; 
			std::uint32_t maxPlayers : 5 = 0; // 8v8 => 16
			std::uint32_t hasMatchStarted : 1 = 0;
			std::uint32_t hasPassword : 1 = 0;
			std::uint32_t isOpen : 1 = 0;  
			std::uint32_t weaponRestriction : 4 = 0; 
			std::uint32_t handlePlayerInvite : 1 = 0;  // if 1 then client crashes?!  
			std::uint32_t isTeamBalanceOn : 1 = 0; 
			std::uint32_t isTeamBalanceOn2 : 1 = 0; // ?
			std::uint32_t isObserverOn : 1 = 0;
			std::uint32_t hidePassword : 1 = 0;
			std::uint32_t unknown1 : 1 = 0;
			std::uint32_t unknown2 : 1 = 0;
			std::uint32_t unknown3 : 1 = 0;
			char password[14]{};
			std::uint8_t unknown4{};
		};
#pragma pack(pop)
	}
}

#endif