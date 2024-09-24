
#ifndef PLAYER_POSITION_FROM_SERVER_H
#define PLAYER_POSITION_FROM_SERVER_H

#include <cstdint>
#include "PlayerPositionFromClient.h"

namespace Cast
{
    namespace Structures
    {

#pragma pack(push, 1)
        struct SpecificInfo
        {
            std::uint32_t sessionId : 14 = 0;
            std::uint32_t enableMovement : 1 = true;
            std::uint32_t enableBullet : 1 = false;
            std::uint32_t animation1 : 7 = 0;
            std::uint32_t enableRotation : 1 = true;
            std::uint32_t animation2 : 6 = 0;
            std::uint32_t handlePlayerInvite : 1 = true;
            std::uint32_t enableJump : 1 = false;
        };
#pragma pack(push, 1)

#pragma pack(push, 1)
        struct PlayerInfoBasicResponse
        {
            std::uint32_t tick{}; // ok
            SpecificInfo specificInfo{}; //ok, check something  // starts at 0
            Cast::Structures::PositionStruct position;   // starts at 4
            Cast::Structures::DirectionStruct direction; // starts at 10
            std::uint32_t rotation1 : 8 = 0;  // starts at 16
            std::uint32_t rotation2 : 8 = 0; // starts at 17
            std::uint32_t rotation3 : 9 = 0; // starts at 18
            std::uint32_t currentWeapon : 4 = 0; // last 3 bits are unused
        };
#pragma pack(pop)

#pragma pack(push, 1)
        struct PlayerInfoResponseWithJump
        {
            PlayerInfoBasicResponse playerInfoBasicResponse;
            Cast::Structures::JumpStruct jump{};
        };
#pragma pack(pop)

#pragma pack(push, 1)
        struct PlayerInfoResponseWithBullets
        {
            std::uint32_t tick{}; // ok
            SpecificInfo specificInfo{}; //ok, check something  // starts at 0
            Cast::Structures::PositionStruct position;   // starts at 4
            Cast::Structures::DirectionStruct direction; // starts at 10
            Cast::Structures::BulletsStruct bullets{}; // starts at 16
            std::uint32_t rotation1 : 8 = 0;  // starts at 24
            std::uint32_t rotation2 : 8 = 0; // starts at 25
            std::uint32_t rotation3 : 9 = 0; // starts at 26
            std::uint32_t currentWeapon : 4 = 0; // last 3 bits are unused
        };
#pragma pack(pop)

#pragma pack(push, 1)
        struct PlayerInfoResponseComplete
        {
            PlayerInfoResponseWithBullets playerInfoBasicResponse;
            Cast::Structures::JumpStruct jump{};
        };
#pragma pack(pop)


    }
}

#endif