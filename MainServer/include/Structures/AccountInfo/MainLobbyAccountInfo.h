#ifndef MAIN_LOBBY_ACCOUNT_INFO_H
#define MAIN_LOBBY_ACCOUNT_INFO_H

#include <cstdint>
#include <chrono>
#include <array>
#include "MainAccountAchievements.h"
#include "MainAccountInfo.h"


namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct LobbyAccountInfo 
		{
			std::uint64_t dioramaInfo{};
			std::uint32_t totalKills{}; 
			std::uint32_t totalDeaths{};
			std::uint32_t totalAssists{};
			std::uint32_t totalWins{};
			std::uint32_t totalLosses{};
			std::uint32_t totalDraws{};
			std::uint32_t meleeKills{};
			std::uint32_t rifleKills{};
			std::uint32_t shotgunKills{};
			std::uint32_t sniperKills{};
			std::uint32_t mgKills{};
			std::uint32_t bazookaKills{};
			std::uint32_t grenadeKills{};
			std::uint64_t unused1 : 27 = 0;
			std::uint64_t headshots : 29 = 0;
			std::uint64_t killstreak : 8 = 0;  
			std::uint32_t unused2{};
			std::uint64_t playtimeSeconds{};
			std::uint32_t unused3{};
			char achievementsUnused[28]{}; // the client remembers the achievements, no need for the server to send this info   //84
			std::uint32_t zombieKills{}; 
			std::uint32_t infected{};
			std::uint32_t unused4{};
			std::array<std::uint32_t, 17> items{};
			std::uint32_t latestCharacterSelected : 4 = 0; 
			std::uint32_t unknown : 4 = 0;
			std::uint32_t unused6 : 9 = 0;
			std::uint32_t level : 7 = 0;  
			std::uint32_t vipExperience{}; // client does vipExperience - 1 to this one 
			std::array<std::uint32_t, 17> unknown2{}; 

			char clanName[16]{};
			std::uint64_t clanLogoFrontId : 16 = 0;
			std::uint64_t clanLogoBackId : 14 = 0;
			std::uint64_t clanContribution : 34 = 0;
			std::uint64_t clanWins : 23 = 0;
			std::uint64_t clanLosses : 23 = 0;
			std::uint64_t clanDraws : 18 = 0;
			std::uint32_t clanKills{};
			std::uint32_t clanDeaths{};
			std::uint32_t clanAssists{};

			explicit LobbyAccountInfo(const Main::Structures::AccountInfo& accountInfo)
				: dioramaInfo{ accountInfo.diorama }, totalKills{ accountInfo.totalKills }, totalDeaths{ accountInfo.deaths }, totalAssists{ accountInfo.assists }
				, totalWins{ accountInfo.wins }, totalLosses{ accountInfo.losses }, totalDraws{ accountInfo.draws }, meleeKills{ accountInfo.meleeKills }
				, rifleKills{ accountInfo.rifleKills }, shotgunKills{ accountInfo.shotgunKills }, sniperKills{ accountInfo.sniperKills }, mgKills{ accountInfo.microgunKills }
				, bazookaKills{ accountInfo.bazookaKills }, grenadeKills{ accountInfo.grenadeKills }, headshots{ accountInfo.headshots }, killstreak{ accountInfo.killstreak }
				, playtimeSeconds{ accountInfo.playtime }, zombieKills{ accountInfo.zombieKills }, level{ static_cast<std::uint32_t>(accountInfo.playerLevel) }
				, latestCharacterSelected{ static_cast<std::uint32_t>(accountInfo.latestSelectedCharacter) }, infected{ accountInfo.infected }, vipExperience{ accountInfo.vipExperience + 1 }
				, clanLogoFrontId{ accountInfo.clanLogoFrontId }, clanLogoBackId{ accountInfo.clanLogoBackId }, clanContribution{ accountInfo.clanContribution }
				, clanWins{ accountInfo.clanWins }, clanLosses{ accountInfo.clanLosses }, clanDraws{ accountInfo.clanDraws }, clanKills{ accountInfo.clanKills }
				, clanDeaths{ accountInfo.clanDeaths }, clanAssists{ accountInfo.clanAssists }
			{
				std::memcpy(clanName, accountInfo.clanName, 16);
			}
		};
#pragma pack(pop)
	}
}

#endif