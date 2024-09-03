#ifndef SINGLE_LOBBY_LIST_H
#define SINGLE_LOBBY_LIST_H

#include "../../../include/Structures/AccountInfo/MainAccountUniqueId.h"

namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
        struct SinglePlayerInfoList
        {
            char name[16]{};
            UniqueId uniqueId{};
            std::uint64_t clanLogoFrontId : 16;
            std::uint64_t clanLogoBackId : 14;
            std::uint64_t level : 7;
        };
#pragma pack(pop)
	}
}


#endif