#ifndef AUTH_INITIAL_PACKET_STRUCTURE_H
#define AUTH_INITIAL_PACKET_STRUCTURE_H

#ifdef WIN32
#include <corecrt.h>
#else
#define __time32_t uint32_t
#endif

#include <ctime>

#include "Protocol/TcpHeader.h"
#include "Protocol/CommandHeader.h"
#include "Enums/MiscellaneousEnums.h"
#include "Enums/ExtrasEnums.h"

namespace Auth
{
	namespace Structures
	{
#pragma pack(push, 1)
		struct ConnectionPacket 
		{
			Common::Protocol::TcpHeader tcpHeader{};
			Common::Protocol::CommandHeader commandHeader{};
			std::int32_t key{};
			__time32_t timestamp{};

			ConnectionPacket()
				: key{ static_cast<std::int32_t>(rand() + 1) }
				, timestamp{ static_cast<__time32_t>(std::time(0)) }
			{
				tcpHeader.initialize(0, Common::Enums::EncryptionType::NO_ENCRYPTION, sizeof(ConnectionPacket));
				commandHeader.initialize(401, Common::Random::generateRandomOption(), static_cast<int>(Common::Enums::AUTH_SUCCESS), Common::Random::generateRandomMission());
			}
		};
#pragma pack(pop)
	}
}

#endif