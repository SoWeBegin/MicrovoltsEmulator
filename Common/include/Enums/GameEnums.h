	#ifndef COMMON_GAME_ENUMS_H
#define COMMON_GAME_ENUMS_H

#include <cstdint>

namespace Common
{
	namespace Enums
	{
		enum Characters
		{
			Naomi = 0,
			Kai = 1,
			Pandora = 2,
			CHIP = 3,
			Knox = 4,
			Simon = 5,
			Amelia = 6,
			Sharkill = 7,
			Sophitia = 8,
		};

		enum Team
		{
			TEAM_ALL = 0,
			TEAM_RED = 1,
			TEAM_BLUE = 2,
			TEAM_OBSERVER = 4,
			TEAM_ZOMBIE = 5
		};

		enum ItemType
		{
			HAIR = 0,
			FACE = 1,
			DRESS = 2,
			LEGS = 3,
			SKIRT = 4,
			GLOVES = 5,
			BOOTS = 6,
			ACC_UPPER = 7,
			ACC_WAIST = 8,
			ACC_BACK = 9,
			MELEE = 10,
			RIFLE = 11,
			SHOTGUN = 12,
			SNIPER = 13,
			MG = 14,
			BAZOOKA = 15,
			GRENADE = 16,
			SET = 17,
		};

		enum PlayerState : std::uint32_t
		{
			STATE_PLAYING = 1,
			//STATE_UNKNOWN2 = 2,
			STATE_SHOP = 3,
			STATE_INVENTORY = 4,
			STATE_CAPSULE = 5,
			STATE_WAITING = 6,
			STATE_READY = 7,
			STATE_PLAY = 11,

			// Tried for "Playing": 8, 9, 10, 11, 12, 13
			//STATE_NORMAL = 9, 
		};

		enum PlayerType
		{
			HOST,
			NON_HOST,
		};
	}
}

#endif