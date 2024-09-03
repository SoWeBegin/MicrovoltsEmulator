#ifndef ROOM_JOIN_LATEST_INFO_H
#define ROOM_JOIN_LATEST_INFO_H

#include <cstdint>

// This struct is sent to the client as the last one when one joins a room
// It includes information (such as some room settings) that weren't sent before, e.g. "specialSetting" and "isItemOn"

namespace Main
{
	namespace Structures
	{
#pragma pack(push,1)
		struct RoomJoinLatestInfo
		{
			// N.B GM grade can't enter observer mode at all seemingly (it's literally disabled)
			std::uint16_t unknown = 0;
			std::uint16_t specialSetting : 5 = 0; // Special Setting: total kills, rounds etc. not sent before
			std::uint16_t unknown1 : 3 = 0;
			std::uint16_t keepThisOne : 1 = 1; // For some reason, the next bit needs this one to be set to '1' to work properly
			std::uint16_t hasMatchStarted : 1 = 0;
			std::uint16_t isItemOn : 1 = 0;
			std::uint16_t time : 5 = 0;
			std::uint32_t weaponRestriction : 4 = 0;
			std::uint32_t unknown2 : 28 = 0;

			RoomJoinLatestInfo(std::uint16_t specialSetting, bool hasMatchStarted, bool isItemOn, std::uint16_t time, std::uint32_t weaponRestriction)
				: specialSetting{ specialSetting }, hasMatchStarted{ hasMatchStarted }, isItemOn{ isItemOn }, time{ time }, weaponRestriction{ weaponRestriction }
			{
			}
		};
#pragma pack(pop)
	}
}

#endif