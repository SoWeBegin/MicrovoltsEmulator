#ifndef MAIN_ACCOUNT_INFO_H
#define MAIN_ACCOUNT_INFO_H

#include <cstdint>
#include <chrono>
#include "Enums/GameEnums.h"
#include "Enums/MiscellaneousEnums.h"
#include "MainAccountUniqueId.h"
#include "MainAccountAchievements.h"


namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct AccountInfo
		{
			std::uint64_t diorama{};  
			std::uint32_t totalKills{}; //8
			std::uint32_t deaths{}; //12
			std::uint32_t assists{};//16
			std::uint32_t wins{};//20
			std::uint32_t losses{};//24
			std::uint32_t draws{};//28
			std::uint32_t meleeKills{};//32
			std::uint32_t rifleKills{};//36
			std::uint32_t shotgunKills{};//40
			std::uint32_t sniperKills{};//44
			std::uint32_t microgunKills{};//48
			std::uint32_t bazookaKills{};//52
			std::uint32_t grenadeKills{};//56
			std::uint64_t unused1 : 27 = 0;//60
			std::uint64_t headshots : 29 = 0;
			std::uint64_t killstreak : 8 = 0;
			std::uint32_t unknown1{};//68
			std::uint32_t playtime{};//72
			std::uint32_t clanId{}; // >= 8  == has clan  //74
			AccountAchievements achievements{}; // Not read from DB currently    // 78
			std::uint32_t zombieKills{}; //110
			std::uint32_t infected{}; //114
			std::uint32_t unknown2{};       //118
			char nickname[16]{}; // cannot contain spaces   // 122
			std::uint64_t serverTime{}; //138
			UniqueId uniqueId{}; //144
			std::uint64_t playerGrade : 4 = 0;//148
			std::uint64_t unused2 : 1 = 0;
			std::uint64_t latestSelectedCharacter : 4 = 0;
			std::uint64_t boughtCharacters : 16 = 0;
			std::uint64_t playerLevel : 7 = 0;
			std::uint64_t coins : 7 = 0;
			std::uint64_t battery : 14 = 0;
			std::uint64_t luckyPoints : 11 = 0;
			std::uint32_t experience{};//156
			std::uint64_t microPoints : 31 = 0;//160
			std::uint64_t rockTotens : 30 = 0;
			std::uint64_t goldenMode : 3 = 0; // if 3 ==> displays Golden Mode message in Lobby
			std::uint32_t isTutorialDone : 22 = 0; //168
			std::uint32_t inventorySpace : 10 = 0;
			std::uint32_t maxBattery : 13 = 0; //172
			std::uint32_t singleWaveAttempts : 9 = 0;
			std::uint32_t highestSinglewaveStage : 10 = 0;
			std::uint32_t highestSingleWaveScore{}; // on client => "invasion" //176
			std::uint32_t unknown3{}; //180
			std::uint32_t userstory{};//192
			std::uint32_t vipExperience{};//196
			std::uint32_t accountID{};    //192
			std::uint32_t accountKey{};//188
			char unused[8]{};//192
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

			std::uint64_t getUtcTimeMs() const
			{
				const auto durationSinceEpoch = std::chrono::system_clock::now().time_since_epoch();
				return static_cast<std::uint64_t>(duration_cast<std::chrono::milliseconds>(durationSinceEpoch).count());
			}

			void setServerTime()
			{
				serverTime = 51877635; // getUtcTimeMs();
			}

		};
#pragma pack(pop)

	}
}

#endif	
