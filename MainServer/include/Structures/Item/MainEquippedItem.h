
#ifndef MAIN_EQUIPPED_ITEM_INFO_H
#define MAIN_EQUIPPED_ITEM_INFO_H

#include <corecrt.h>
#include <cstdint>
#include "MainItemSerialInfo.h"
#include <array>

namespace Main
{
	namespace Structures
	{
		class Item;
#pragma pack(push, 1)
		struct EquippedItem
		{
			std::uint32_t type : 8 = 0; 
			std::uint32_t id : 24 = 0;    
			__time32_t expirationDate{}; 
			ItemSerialInfo serialInfo{};
			std::uint16_t durability{}; 
			std::uint16_t energy{};
			std::uint32_t isSealed{};
			std::uint32_t sealLevel{};
			std::uint32_t experienceEnhancement{};
			std::uint32_t mpEnhancement{};

			EquippedItem() = default;

			explicit EquippedItem(const Item& item);
		};
#pragma pack(pop)


#pragma pack(push, 1)
		struct DetailedEquippedItem : EquippedItem
		{
			std::uint16_t characterId{};

			DetailedEquippedItem() = default;

			explicit DetailedEquippedItem(const Item& item, std::uint16_t charId)
				: EquippedItem{ item }, characterId{ charId }
			{
			}

			explicit DetailedEquippedItem(const EquippedItem& equippedItem, std::uint16_t charId)
				: EquippedItem{ equippedItem }, characterId{ charId }
			{
			}
		};
#pragma pack(pop)
	
#pragma pack(push, 1)
		struct BasicEquippedItem
		{
		private:
			struct Data
			{
				std::uint32_t type : 8 = 0;
				std::uint32_t equippedItemId : 24 = 0;    
				std::uint32_t unknown = 0;

				Data(const Main::Structures::EquippedItem& equippedItem)
					: equippedItemId{ equippedItem.id }, type{ equippedItem.type }
				{
				}

				Data() = default;
			};

		public:
			std::array<Data, 7> equippedWeapons{};
			std::array<Data, 10> equippedItems{};
		};
#pragma pack(pop)
	}
}

#endif
