#ifndef SPAWNED_ITEM_H
#define SPAWNED_ITEM_H

#include <cstdint>
#include "MainItemSerialInfo.h"
#include "ConstantDatabase/Cdb.h"
#include "../../MainEnums.h"
#include "ConstantDatabase/CdbSingleton.h"
#include "ConstantDatabase/Structures/CdbItemInfo.h"
#include "ConstantDatabase/Structures/CdbWeaponsInfo.h"
#include "../../CdbUtils.h"

// Note: Creation date cannot be 0 (otherwise the client doesn't know how to handle equipping/unequipping) !!!
namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct SpawnedItem
		{
			std::uint32_t itemId{};
			std::uint32_t expirationDate{};  
			Main::Structures::ItemSerialInfo serialInfo{};
			Main::Enums::ItemFrom itemFrom = Main::Enums::ItemFrom::GIFT;

			SpawnedItem()
			{
				serialInfo.itemCreationDate = static_cast<__time32_t>(std::time(0));
				serialInfo.itemOrigin = Main::Enums::ItemFrom::GIFT;
				serialInfo.m_serverId = 0;
			}
		};
#pragma pack(pop)

#pragma pack(push, 1)
		struct BoxItem
		{
			std::uint32_t itemId{};
			std::uint32_t expirationDate{};
			Main::Structures::ItemSerialInfo serialInfo{};

			BoxItem()
			{
				serialInfo.itemCreationDate = static_cast<__time32_t>(std::time(0));
				serialInfo.m_serverId = 0;
			}
		};
#pragma pack(pop)
	}
}

#endif

