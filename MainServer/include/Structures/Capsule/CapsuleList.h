#ifndef CAPSULE_LIST_STRUCT_H
#define CAPSULE_LIST_STRUCT_H

#include <cstdint>

namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct CapsuleList
		{
			std::uint32_t capsuleInfoId{};
			//std::uint32_t price{};
			//std::uint32_t unknown1{}; // probably for events, timestamp
			//std::uint32_t unknown2{}; // probably for events, timestamp
		};
#pragma pack(pop)
	}
}

#endif