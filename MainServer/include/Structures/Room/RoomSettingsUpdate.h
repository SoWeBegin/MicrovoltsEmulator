#ifndef ROOM_SETTINGS_UPDATE_H
#define ROOM_SETTINGS_UPDATE_H

#include <cstdint>

namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct RoomSettingsUpdateBase
		{
			std::uint32_t maxPlayers : 5 = 0; // ok
			std::uint32_t time : 5 = 0; // ok
			std::uint32_t weaponRestriction : 4 = 0; // ok
			std::uint32_t isItemOn : 1 = 0; // ok
			std::uint32_t isOpen : 1 = 0; // ok
			std::uint32_t map : 6 = 0; // ok
			std::uint32_t unknown1 : 1 = 0;
			std::uint32_t unknown2 : 1 = 0;
			std::uint32_t specificSetting : 5 = 0; // ok
			std::uint32_t isTeamBalanceOn : 1 = 0; // ok
			std::uint32_t unknown3 : 1 = 0;
			std::uint32_t unknown4 : 1 = 0;
		};
#pragma pack(pop)

#pragma pack(push, 1)
		struct RoomSettingsUpdateTitle
		{
			RoomSettingsUpdateBase roomSettingsUpdateBase;
			char title[30]{};
			char padding[2]{};
		};
#pragma pack(pop)

#pragma pack(push, 1)
		struct RoomSettingsUpdatePassword
		{
			RoomSettingsUpdateBase roomSettingsUpdateBase;
			char password[8]{};
			char padding[8]{};
		};
#pragma pack(pop)

#pragma pack(push, 1)
		struct RoomSettingsUpdateTitlePassword
		{
			RoomSettingsUpdateBase roomSettingsUpdateBase;
			char title[30]{};
			char padding[2]{};
			char password[8]{};
			char padding2[8]{};
		};
#pragma pack(pop)
	}
}

#endif