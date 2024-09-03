#ifndef AUTH_ACCOUNTINFO_H
#define AUTH_ACCOUNTINFO_H

#include <stdlib.h>
#include "Utils/RandomGeneration.h"
#include "Enums/MiscellaneousEnums.h"

namespace Auth
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct BasicAccountInfo
		{
			std::uint32_t accountId{};
			std::uint32_t hashKey{};
			std::uint32_t level{};
			std::uint32_t exp{};
			std::uint32_t kills{};
			std::uint32_t deaths{};
			std::uint32_t assists{};
			std::uint32_t wins{};
			std::uint32_t losses{};
			std::uint32_t draws{};
			char playerName[16]{};
			std::uint16_t clanIconFrontID{};
			std::uint16_t clanIconBackID{};
			char clanName[20]{};
			std::uint32_t vipInfo{}; // different clients have this

			// commandHeader.option = player grade
			// commandHeader.extra  = login type 
		};
#pragma pack(pop)
	}
}

#endif