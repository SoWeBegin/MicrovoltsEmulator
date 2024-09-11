#ifndef END_SCOREBOARD_STRUCT_H
#define END_SCOREBOARD_STRUCT_H

#include <cstdint>
#include "AccountInfo/MainAccountUniqueId.h"

namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct ClientEndingMatchNotification
		{
			std::uint32_t meleeKills : 8 = 0;
			std::uint32_t rifleKills : 8 = 0;
			std::uint32_t shotgunKills : 8 = 0;
			std::uint32_t sniperKills : 8 = 0;
			std::uint32_t mgKills : 8 = 0;
			std::uint32_t bazookaKills : 8 = 0;
			std::uint32_t grenadeKills : 8 = 0;
			std::uint32_t killstreak : 8 = 0;
			std::uint32_t totalKills : 8 = 0;
			std::uint32_t totalDeaths : 8 = 0;
			std::uint32_t headshots : 8 = 0;
			std::uint32_t assists : 8 = 0;
			std::uint32_t unknown2 = 0;
			Main::Structures::UniqueId uniqueId{};
		};
#pragma pack(pop)

#pragma pack(push, 1)
		struct ScoreboardResponse
		{
			std::uint32_t meleeKills : 8;
			std::uint32_t rifleKills : 8;
			std::uint32_t shotgunKills : 8;
			std::uint32_t sniperKills : 8;
			std::uint32_t mgKills : 8;
			std::uint32_t bazookaKills : 8;
			std::uint32_t grenadeKills : 8;
			std::uint32_t probablyKillStreak : 8;
			std::uint32_t totalKills : 8;
			std::uint32_t deaths : 8;
			std::uint32_t headshots : 8;
			std::uint32_t assists : 8;
			std::uint32_t u0 = 0;
			std::uint32_t newTotalMP = 0;
			std::uint32_t newTotalEXP = 0; 
			std::uint32_t u4 = 0;

			ScoreboardResponse(const Main::Structures::ClientEndingMatchNotification& finalScoreGivenByClient)
				: meleeKills{ finalScoreGivenByClient.meleeKills }, rifleKills{ finalScoreGivenByClient.rifleKills }, shotgunKills{ finalScoreGivenByClient.shotgunKills },
				sniperKills{ finalScoreGivenByClient.sniperKills }, mgKills{ finalScoreGivenByClient.mgKills }, bazookaKills{ finalScoreGivenByClient.bazookaKills },
				grenadeKills{ finalScoreGivenByClient.grenadeKills }, probablyKillStreak{ finalScoreGivenByClient.killstreak }, totalKills{ finalScoreGivenByClient.totalKills },
				deaths{ finalScoreGivenByClient.totalDeaths }, headshots{ finalScoreGivenByClient.headshots }, assists{ finalScoreGivenByClient.assists }
			{
			}
		};
#pragma pack(pop)

	}
}

#endif
