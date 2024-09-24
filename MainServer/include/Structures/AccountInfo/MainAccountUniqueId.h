#ifndef MAIN_ACOUNT_UNIQUE_ID_H
#define MAIN_ACOUNT_UNIQUE_ID_H

#include <cstdint>


namespace Main
{
    namespace Structures
    {
#pragma pack(push, 1)
        struct UniqueId
        {
            std::uint32_t session : 16 = 0; 
            std::uint32_t server : 15 = 0; 
            std::uint32_t handlePlayerInvite : 1 = 0; 

            bool operator==(const UniqueId& other) const = default;
        };
#pragma pack(pop)
    }
}

#endif