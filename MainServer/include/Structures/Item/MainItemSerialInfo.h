#ifndef MAIN_ITEM_SERIALINFO_H
#define MAIN_ITEM_SERIALINFO_H

#include <cstdint>
#include <ctime>
#include <compare>

#ifdef WIN32
#include <corecrt.h>
#else
#define __time32_t uint32_t
#endif

namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct ItemSerialInfo
		{
			/*
			std::uint64_t itemNumber : 16 = 0;
			std::uint64_t itemOrigin : 8 = 0; // 0 = shop, 1 = gift, 4 = event, 5 = dev tool, 6 = web shop, 8 = gm spawn
			std::uint64_t m_serverId : 8 = 0;
			std::uint64_t itemCreationDate : 32 = 0;
			*/
			
			std::uint64_t itemNumber : 20 = 0;
			std::uint64_t m_serverId : 4 = 0;
			std::uint64_t handlePlayerInvite : 4 = 0;
			std::uint64_t itemOrigin : 4 = 0; 
			std::uint64_t itemCreationDate : 32 = 0;
			

			auto operator<=>(const ItemSerialInfo&) const = default;

			ItemSerialInfo()
			{
				itemCreationDate = static_cast<__time32_t>(std::time(0)); // absolutely necessary, can't be 0 otherwise the client doesn't know how to handle
				// equipping/unequipping of items!
			}
		};
#pragma pack(pop)
	}
}

#endif