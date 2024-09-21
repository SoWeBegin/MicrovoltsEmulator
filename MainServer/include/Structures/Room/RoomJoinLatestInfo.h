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
        struct ModeInfoTDM
        {
            std::uint64_t redscore : 8 = 0;
            std::uint64_t bluescore : 8 = 0;
            std::uint64_t winrule : 8 = 0;
            std::uint64_t state : 2 = 0;
            std::uint64_t kitdrop : 1 = 1;
            std::uint64_t timelimited : 5 = 0;
            std::uint64_t weaponlimited : 4 = 0;
        };
#pragma pack(pop)

#pragma pack(push,1)
        struct ModeInfoFFA
        {
            std::uint64_t timelimited : 5 = 0;
            std::uint64_t winrule : 5 = 0;
            std::uint64_t team_balance : 1 = 0;
            std::uint64_t state : 2 = 3;
            std::uint64_t weaponlimited : 4 = 0;
        };
#pragma pack(pop)

#pragma pack(push,1)
        struct ModeInfoScrimmage
        {
            std::uint64_t unknown : 45 = 0;
            std::uint64_t state : 2 = 3;
            std::uint64_t timelimited : 5 = 0;
            std::uint64_t weaponlimited : 4 = 0;
        };
#pragma pack(pop)
	}
}

#endif