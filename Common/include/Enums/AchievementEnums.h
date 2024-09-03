#ifndef COMMON_ACHIEVEMENT_ENUMS_H
#define COMMON_ACHIEVEMENT_ENUMS_H

namespace Common
{
	namespace Enums
	{
		enum AchievementType
		{
			// Each value represents the bit position where the achievement starts.
			// For example, these values are represented with the bit '0' here:
			// 00011110 11110111 10111101 11101111 01111011 11111110
			TIER_BATTLE = 0,
			TIER_MELEE = 10,
			TIER_RIFLE = 15,
			TIER_SHOTGUN = 20,
			TIER_SNIPER = 25,
			TIER_MG = 30,
			TIER_BAZOOKA = 35,
			TIER_GRENADE = 40
		};
	}
}

#endif