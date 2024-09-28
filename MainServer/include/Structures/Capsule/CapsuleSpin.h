#ifndef CAPSULE_SPIN_STRUCT_H
#define CAPSULE_SPIN_STRUCT_H

#include <cstdint>
#include "../Item/MainItemSerialInfo.h"
#include "../../MainEnums.h"

namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct CapsuleSpin
		{
			std::uint32_t winItemId{};
			std::uint32_t mpAmount{};
			Main::Structures::ItemSerialInfo itemSerialInfo{};

			CapsuleSpin()
			{
				itemSerialInfo.itemCreationDate = static_cast<__time32_t>(std::time(0));
				itemSerialInfo.itemOrigin = Main::Enums::ItemFrom::GIFT;
				itemSerialInfo.m_serverId = 0;
			}
		};
#pragma pack(pop)
	}
}

#endif