#include "../../../include/Structures/Item/MainItem.h"
#include "../../../include/CdbUtils.h"
#include "../../../include/Structures/Item/MainEquippedItem.h"
#include "Enums/GameEnums.h"

namespace Main
{
	namespace Structures
	{
		EquippedItem::EquippedItem(const Main::Structures::Item& item)
			: id{ item.id << 1 }, expirationDate{ item.expirationDate }, serialInfo{ item.serialInfo }, durability{ item.durability }
			, energy{ item.energy }, isSealed{ item.isSealed }, sealLevel{ item.sealLevel }, experienceEnhancement{ item.experienceEnhancement }
			, mpEnhancement{ item.mpEnhancement }
		{
			type = Main::ConstantDatabase::CdbUtil::getItemType(item.id);
			if (type >= 17)
			{
				type = Common::Enums::ItemType::SET; // Sets
			}
		}
	}
}

