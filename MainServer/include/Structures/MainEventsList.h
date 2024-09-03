#ifndef MAIN_EVENTS_LIST_H
#define MAIN_EVENTS_LIST_H

#include <cstdint>
#include "AccountInfo/MainAccountUniqueId.h"
#include "Enums/RoomEnums.h"
#include <vector>

namespace Main
{
    namespace Structures
    {
#pragma pack(push, 1)
        struct SingleModeEvent
        {
            Common::Enums::GameModes gameMode{};
            __time32_t startDate{};
            __time32_t endDate{};
        };
#pragma pack(pop)



#pragma pack(push, 1)
        struct SingleMapEvent
        {
            Common::Enums::GameMaps gameMap{};
            __time32_t startDate{};
            __time32_t endDate{};
        };
#pragma pack(pop)
    }
}

#endif