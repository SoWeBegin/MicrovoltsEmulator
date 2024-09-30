
#ifndef MAIN_ITEM_INFO_H
#define MAIN_ITEM_INFO_H

#include <cstdint>
#include "ConstantDatabase/CdbSingleton.h"
#include "ConstantDatabase/Structures/CdbItemInfo.h"
#include "ConstantDatabase/Structures/CdbWeaponsInfo.h"
#include "MainItemSerialInfo.h"
#include "../TradeSystem/TradeSystemItem.h"
#include "SpawnedItem.h"

#include "../../CdbUtils.h"

#include "../../Structures/Item/MainBoughtItem.h"
#include "../../Structures/Item/MainEquippedItem.h"

namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct Item
		{
			// note: If itemnumber = 0 AND creationDate = 0 ==> basic item
			std::uint32_t id{};
			// Adding the following to the base ID upgrades the weapon as follows:
			// Add +[1-9] => firing rate upgrade level
			// Add +[11-19] => power upgrade level
			// Add +[21-29] => projectile speed level
			__time32_t expirationDate{}; 
			ItemSerialInfo serialInfo{};
			std::uint16_t durability{}; 
			std::uint16_t energy{};
			std::uint32_t isSealed{};
			std::uint32_t sealLevel{}; // num seals needed for a given item
			std::uint32_t experienceEnhancement{}; // No idea what this does - NEEDS TESTING WHEN CAST SERVER READY
			std::uint32_t mpEnhancement{};  // No idea what this does - NEEDS TESTING WHEN CAST SERVER READY
			std::uint32_t unknown{ 1 };

			Item() = default;

			Item(const Main::Structures::BoughtItem& boughtItem)
				: id{ boughtItem.itemId }, serialInfo{ boughtItem.serialInfo }
			{
				Main::ConstantDatabase::CdbUtil cdbUtil(boughtItem.itemId);

				// Failure here is exceptional, let the caller handle it
				expirationDate = *(cdbUtil.getItemDuration());
				durability = *(cdbUtil.getItemDurability());
			}

			explicit Item(const Main::Structures::EquippedItem& equippedItem)
				: id{ (uint32_t)equippedItem.id >> 1}, serialInfo{ equippedItem.serialInfo }, expirationDate{ equippedItem.expirationDate }
				, durability{ equippedItem.durability }, isSealed{ equippedItem.isSealed }, sealLevel{ equippedItem.sealLevel }
				, experienceEnhancement{ equippedItem.experienceEnhancement }, mpEnhancement{ equippedItem.mpEnhancement }
				, energy{ equippedItem.energy }
			{
			}

			Item(const Main::Structures::TradeBasicItem& tradedItem)
				: id{ tradedItem.itemId }, serialInfo{ tradedItem.itemSerialInfo }
			{
				// change item origin to ORIGIN_TRADE
				Main::ConstantDatabase::CdbUtil cdbUtil(tradedItem.itemId);
				expirationDate = 0; // trade only works with unlimited items
				durability = *(cdbUtil.getItemDurability());
			}

			Item(const Main::Structures::SpawnedItem& spawnedItem)
				: id{ spawnedItem.itemId }, serialInfo{ spawnedItem.serialInfo }
			{
				Main::ConstantDatabase::CdbUtil cdbUtil(spawnedItem.itemId);
				expirationDate = spawnedItem.expirationDate; 
				durability = *(cdbUtil.getItemDurability());
			}

		};
#pragma pack(pop)
	}
}

#endif
