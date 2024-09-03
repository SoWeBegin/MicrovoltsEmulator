#ifndef ENUMS_COMMON_ROOM_H
#define ENUMS_COMMON_ROOM_H

#include <cstdint>

namespace Common
{
	namespace Enums
	{
		enum GameModes : std::uint32_t
		{
			TeamDeathMatch = 0,
			FreeForAll = 1,
			ItemMatch = 2,
			CaptureTheBattery = 3,
			CloseCombat = 4,
			Elimination = 5,
			SuperItemMatch = 6,
			ZombieMode = 7,
			ArmsRace = 8,
			Scrimmage = 9,
			BombBattle = 10,
			SniperMode = 11,
			SquareMode = 12,
			BossBattle = 13,
			AiBattle = 14
		};

		enum RoomChangeHostExtra
		{
			CHANGE_HOST_SUCCESS = 1,
			CHANGE_HOST_FAIL = 2, // Unknown error. Please restart the client
			CHANGE_HOST_DOESNT_EXIST = 0xD,
			CHANGE_HOST_NO_PERMISSION = 0x10,
		};

		enum WeaponRestriction : std::uint32_t
		{
			MeleeOnly = 0,
			RifleOnly = 1,
			ShotgunOnly = 2,
			SniperOnly = 3,
			GatlingOnly = 4,
			BazookaOnly = 5,
			GrenadeOnly = 6,
			All = 7,
			WeaponSelect = 9,
		};

		enum GameMaps : std::uint32_t
		{
			Random = 0,
			Chess = 1,
			ToyFleet = 2,
			TrackersFactory = 3,
			Beach = 4,
			BattleMine = 5,
			ToyGarden = 6,
			Neighboorhood = 7,
			MagicPaperLand = 8,
			HobbyShop = 9,
			Academy = 10,
			TheStudio = 11,
			PVCFactory = 13,
			HouseTop = 14,
			WildWest = 15,
			RumpusRoom = 16,
			Cargo = 17,
			PVCFactoryDark = 18,
			ForgottenJunkYard = 19,
			JunkYard = 20,
			GothicCastle = 21,
			Bitmap = 22,
			RockBand = 23,
			ModelShip = 27,
			AcademyInvasion = 28,
			Football = 29,
			Bitmap2 = 30,
			ToyGarden2 = 31,
			RockBandS = 32,
			RockBandW = 33,
			CastleSiege = 34,
			TempleRuins = 35,
			BitmapPlant = 36,
			TheAftermath = 37,
			SinglePlayerEasy = 43,
			SinglePlayerHard = 44,
			BossBattleMap = 45,
			SquareModeMap = 46,
			SquareModeMap2 = 47
		};

        enum RoomSettings : std::uint32_t
        {
            // FFA, TDM, ItemMatch, SIM, SniperMode, CloseCombat
            Kills10 = 0x21, //A.I only
            Kills20 = 0x22,
            Kills30 = 0x23,
            Kills40 = 0x24, //FFA only
            Kills50 = 0x25,
            Kills80 = 0x28,
            Kills100 = 0x2A,
            Kills130 = 0x2D,
            Kills150 = 0x2F,

            // Arms Race
            Points20 = 0x34,

            // Elimination, ZombieMode, BombBattle
            Rounds3 = 0x23,
            Rounds5 = 0x25,
            Rounds7 = 0x27,
            Rounds9 = 0x29,

            // Square
            NoGoals = 0x21,

            // Scrimmage
            HP10000 = 0x21,
            HP20000 = 0x22,
            HP30000 = 0x23,

            // BossBattle
            BossCleared = 0x21,

            // CTB
            Points3 = 0x23,
            Points5 = 0x25,
            Points7 = 0x27,
            Points9 = 0x29
        };
	}
}

#endif