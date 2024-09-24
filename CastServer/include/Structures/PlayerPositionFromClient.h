#ifndef PLAYER_POSITION_STRUCTURE_H
#define PLAYER_POSITION_STRUCTURE_H

#include <cstdint>

#include "DirectXPackedVector.h"


namespace Cast
{
	namespace Structures
	{
#pragma pack(push, 1)
        struct PositionStruct
        {
            DirectX::PackedVector::HALF positionX{};
            DirectX::PackedVector::HALF positionY{};
            DirectX::PackedVector::HALF positionZ{};
        };
#pragma pack(pop)

#pragma pack(push, 1)
        struct DirectionStruct
        {
            DirectX::PackedVector::HALF directionX{};
            DirectX::PackedVector::HALF directionY{};
            DirectX::PackedVector::HALF directionZ{};
        };
#pragma pack(pop)

#pragma pack(push, 1)
        struct BulletsStruct
        {
            DirectX::PackedVector::HALF bullet1{};
            DirectX::PackedVector::HALF bullet2{};
            DirectX::PackedVector::HALF bullet3{};
            DirectX::PackedVector::HALF bullet4{};
        };
#pragma pack(pop)

#pragma pack(push, 1)
        struct JumpStruct
        {
            DirectX::PackedVector::HALF jump1;
            DirectX::PackedVector::HALF jump2;
        };
#pragma pack(pop)

        
#pragma pack(push, 1)
        struct ClientPlayerInfoBasic
        {
            PositionStruct position;
            DirectionStruct direction;
            std::uint32_t matchTick{}; 
            std::uint32_t animation1 : 7 = 0;
            std::uint32_t animation2 : 6 = 0;
            std::uint32_t weapon : 4 = 0;
            std::uint32_t rotation : 9 = 0;
            std::uint32_t handlePlayerInvite : 6 = 0;
        };
#pragma pack(pop)

#pragma pack(push, 1)
        struct ClientPlayerInfoJump
        {
            ClientPlayerInfoBasic playerPositionBasic;
            JumpStruct jumpStruct{};
        };
#pragma pack(pop)

#pragma pack(push, 1)
        struct ClientPlayerInfoBullet
        {
            ClientPlayerInfoBasic playerPositionBasic;
            BulletsStruct bulletStruct{};
        };
#pragma pack(pop)
	
#pragma pack(push, 1)
        struct ClientPlayerInfoComplete
        {
            ClientPlayerInfoBasic playerPositionBasic;
            BulletsStruct bulletStruct{};
            JumpStruct jumpStruct{};
        };
#pragma pack(pop)
    }
}
#endif