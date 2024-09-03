#ifndef TCP_PACKET_HEADER_H
#define TCP_PACKET_HEADER_H

#include <cstdint>
#include <cstdlib>

namespace Common
{
	namespace Protocol
	{
#pragma pack(push, 1)
		class TcpHeader
		{
		private:
			std::uint32_t bogus : 4 = 0;          // Useless, probably for padding
			std::uint32_t sessionId : 14 = 0;     // To distinguish receiver
			std::uint32_t size : 11 = 0;          // Total size of packet, header included [min:4; max:2047]
			std::uint32_t crypt : 3 = 0;          // Cryptography type

		public:
			constexpr TcpHeader() = default;

			explicit TcpHeader(std::uint32_t data);

			void initialize(std::uint32_t sessionId, std::uint32_t crypt, std::uint32_t size);

			void setBogus(std::uint32_t p);

			void setSessionId(std::uint32_t s);

			void setSize(std::uint32_t s);

			void setCrypt(std::uint32_t c);

			std::uint32_t getData() const;

			std::uint32_t getBogus() const;

			std::uint32_t getSessionId() const;

			std::uint32_t getSize() const;

			std::uint32_t getCrypt() const;
		};
#pragma pack(pop)

	}
}

#endif