#ifndef MAILBOX_STRUCTURE_H
#define MAILBOX_STRUCTURE_H

#include "AccountInfo/MainAccountUniqueId.h"


namespace Main
{
	namespace Structures
	{
#pragma pack(push, 1)
        struct Mailbox
        {
            std::uint32_t accountId{};
            std::uint32_t timestamp{};
            std::uint32_t hasBeenRead{};
            char nickname[16]{};
            char message[256]{};
        };
#pragma pack(pop)

#pragma pack(push, 1)
        struct Giftbox
        {
            std::uint32_t accountId{};
            std::uint32_t timestamp{};
            std::uint32_t unknown1;
            std::uint64_t unknown2;
            char nickname[16]{};
            char message[256]{};
        };
#pragma pack(pop)
    }
}
	

#endif