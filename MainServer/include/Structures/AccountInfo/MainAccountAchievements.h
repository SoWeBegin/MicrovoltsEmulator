#ifndef MAIN_ACCOUNT_ACHIEVEMENTS_H
#define MAIN_ACCOUNT_ACHIEVEMENTS_H

#include "Enums/AchievementEnums.h"
#include <cstdint>

namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct AccountAchievements
		{
			// Maybe create new DB table for achievements?
			std::uint64_t achievementsTier1{};
			std::uint64_t achievementsTier2{};
			std::uint64_t achievementsTier3{};
			std::uint64_t achievementsTier4{};


			template <Common::Enums::AchievementType achievementType, typename... Achievements>
			std::uint64_t getDesiredAchievements(Achievements... achievements) const
			{
				static_assert((std::convertible_to<std::size_t, Achievements> && ...),
					"All achievements must have the same type (std::size_t).");

				if constexpr (achievementType == Common::Enums::AchievementType::TIER_BATTLE)
				{
					assert(((achievements > 0 && achievements <= 9) && ...) &&
						"All achievements for TIER_BATTLE must be > 0 and <= 9!");
				}
				else
				{
					assert(((achievements > 0 && achievements <= 4) && ...) &&
						"All achievements for other types must be > 0 and <= 4!");
				}

				std::uint64_t ret = 0;
				((ret |= (static_cast<std::uint64_t>(1) << (achievements + static_cast<int>(achievementType)))), ...);
				return ret;
			}
		};
#pragma pack(pop)
	}
}

#endif