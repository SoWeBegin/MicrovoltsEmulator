#ifndef MAIN_ACCOUNT_INFO_H
#define MAIN_ACCOUNT_INFO_H

#include <cstdint>
#include <chrono>
#include "Enums/GameEnums.h"
#include "Enums/MiscellaneousEnums.h"
#include "MainAccountUniqueId.h"
#include "MainAccountAchievements.h"

#ifndef WIN32
#define __time32_t uint32_t
#endif

namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct AccountInfo
		{
			std::uint64_t diorama{}; 
			std::uint32_t totalKills{}; 
			std::uint32_t deaths{}; 
			std::uint32_t assists{};
			std::uint32_t wins{};
			std::uint32_t losses{};
			std::uint32_t draws{};
			std::uint32_t meleeKills{};
			std::uint32_t rifleKills{};
			std::uint32_t shotgunKills{};
			std::uint32_t sniperKills{};
			std::uint32_t microgunKills{};
			std::uint32_t bazookaKills{};
			std::uint32_t grenadeKills{};
			std::uint64_t unused1 : 27 = 0;
			std::uint64_t headshots : 29 = 0;
			std::uint64_t killstreak : 8 = 0;
			std::uint32_t unknown1{};
			std::uint32_t playtime{};
			std::uint32_t clanId{}; // >= 8  == has clan
			AccountAchievements achievements{}; // Not read from DB currently  
			std::uint32_t zombieKills{};
			std::uint32_t infected{};
			std::uint32_t unknown2{};       
			char nickname[16]{}; // cannot contain spaces   
			std::uint64_t serverTime{};
			UniqueId uniqueId{};
			std::uint64_t playerGrade : 4 = 0;
			std::uint64_t unused2 : 1 = 0;
			std::uint64_t latestSelectedCharacter : 4 = 0;
			std::uint64_t boughtCharacters : 16 = 0;
			std::uint64_t playerLevel : 7 = 0;
			std::uint64_t coins : 7 = 0;
			std::uint64_t battery : 14 = 0;
			std::uint64_t luckyPoints : 11 = 0;
			std::uint32_t experience{};
			std::uint64_t microPoints : 31 = 0;
			std::uint64_t rockTotens : 30 = 0;
			std::uint64_t goldenMode : 3 = 0; // if 3 ==> displays Golden Mode message in Lobby
			std::uint32_t isTutorialDone : 22 = 0;
			std::uint32_t inventorySpace : 10 = 0;
			std::uint32_t maxBattery : 13 = 0;
			std::uint32_t singleWaveAttempts : 9 = 0;
			std::uint32_t highestSinglewaveStage : 10 = 0;
			std::uint32_t highestSingleWaveScore{}; // on client => "invasion"
			std::uint32_t unknown3{}; 
			std::uint32_t userstory{};
			std::uint32_t vipExperience{};
			std::uint32_t accountID{};    
			std::uint32_t accountKey{};
			char unused[8]{};
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

			constexpr void setBoughtCharacters(const std::vector<Common::Enums::Characters>& characters)
			{
				for (const auto& character : characters)
				{
					boughtCharacters |= (static_cast<std::uint64_t>(1) << character);
				}
			}

			constexpr std::uint32_t setZombieKills(std::uint32_t desired)
			{
				return zombieKills = desired * 3; 
			}

			void setServerTime()
			{
				serverTime = __time32_t(std::time(nullptr));
			}

		};
#pragma pack(pop)

	}
}

#endif