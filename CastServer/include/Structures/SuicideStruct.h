
#ifndef SUICIDE_STRUCT_H
#define SUICIDE_STRUCT_H

#include <cstdint>
#include "../../../MainServer/include/Structures/AccountInfo/MainAccountUniqueId.h"


namespace Cast
{
    namespace Structures
    {
#pragma pack(push, 1)
        struct SuicideStructure
        {
            std::uint16_t posX;
            std::uint16_t posY;
            std::uint16_t posZ;
            std::uint32_t idk = 0; // one of these is how much damage taken
            std::uint16_t idk1 = 0;
            Main::Structures::UniqueId uniqueId;
            std::uint32_t newHp = 0;

        };
#pragma pack(pop)
    }
}

#endif