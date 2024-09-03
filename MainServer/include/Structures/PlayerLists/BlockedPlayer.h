#ifndef BLOCKED_PLAYER_STRUCTURE_H
#define BLOCKED_PLAYER_STRUCTURE_H

#include <cstdint>

namespace Main
{
    namespace Structures
    {
#pragma pack(push, 1)
        struct BlockedPlayer
        {
            std::uint32_t targetAccountId{};
            char targetNickname[16];
        };
#pragma pack(pop)
    }
}

#endif