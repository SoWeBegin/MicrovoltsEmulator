#ifndef TRADE_PLAYER_INFO_H
#define TRADE_PLAYER_INFO_H

#include <cstdint>

// Whenever one player adds an item to the trade system this structure is used
namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct TradePlayerInfo
		{
			std::uint32_t unused{};
			std::uint32_t accountId{};
			std::uint32_t characterId{};
			std::uint32_t equippedHair{};
			std::uint32_t equippedEyes{};

			explicit TradePlayerInfo(std::uint32_t accountId, std::uint32_t characterId, std::uint32_t equippedHair, std::uint32_t equippedEyes)
				: accountId{ accountId }, characterId{ characterId }, equippedHair{ equippedHair >> 1 }, equippedEyes{ equippedEyes >> 1 }
			{
			}
		};
#pragma pack(pop)
	}
}

#endif