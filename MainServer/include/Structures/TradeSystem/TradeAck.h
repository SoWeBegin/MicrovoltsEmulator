#ifndef TRADE_ACK_STRUCTURE_H
#define TRADE_ACK_STRUCTURE_H

#include <cstdint>
#include "../AccountInfo/MainAccountUniqueId.h"

namespace Main	
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct TradeAck
		{
			UniqueId uniqueId{};
			std::uint32_t accountId{};
		};
#pragma pack(pop)
	}
}

#endif