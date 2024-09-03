#ifndef BOUGHT_ITEM_H
#define BOUGHT_ITEM_H

#include <cstdint>
#include "../AccountInfo/MainAccountUniqueId.h"
#include "MainItemSerialInfo.h"
#include "ConstantDatabase/Cdb.h"
#include "../../MainEnums.h"
#include "ConstantDatabase/CdbSingleton.h"
#include "ConstantDatabase/Structures/CdbItemInfo.h"
#include "ConstantDatabase/Structures/CdbWeaponsInfo.h"

namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct BoughtItem
		{
			std::uint32_t itemId{};
			std::uint32_t unknown = 0;  // 2==bomb, 3==expired, 0==unlimited, etc.
			ItemSerialInfo serialInfo{};

		public:
			explicit BoughtItem(std::uint32_t itemId)
			{
				this->itemId = itemId - 8388608; // magic value, no idea about the rationale...
				serialInfo.itemCreationDate = static_cast<__time32_t>(std::time(0));
				serialInfo.itemOrigin = Main::Enums::ItemFrom::SHOP;
				serialInfo.m_serverId = 0; // hardcoded for now				
			}
		};
#pragma pack(pop)

#pragma pack(push, 1)
		struct BoughtItemToProlong
		{
			ItemSerialInfo serialInfo{};
			std::uint32_t unknown = 1;  

		public:
			BoughtItemToProlong()
			{
				serialInfo.itemCreationDate = static_cast<__time32_t>(std::time(0));
				serialInfo.itemOrigin = Main::Enums::ItemFrom::SHOP;
				serialInfo.m_serverId = 0; // hardcoded for now				
			}
		};
#pragma pack(pop)
	}
}

#endif

