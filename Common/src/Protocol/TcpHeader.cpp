
#include "../../include/Protocol/TcpHeader.h"
#include <include/Utils/RandomGeneration.h>

namespace Common
{
	namespace Protocol
	{
		TcpHeader::TcpHeader(std::uint32_t data)
		{
			bogus = data & 0xF;
			sessionId = (data >> 4) & 0x3FFF;
			size = (data >> 18) & 0x7FF;
			crypt = (data >> 29) & 0x7;
		}

		void TcpHeader::initialize(std::uint32_t sessionId, std::uint32_t crypt, std::uint32_t size)
		{
			setBogus(0);
			setSessionId(sessionId);
			setCrypt(0 /*crypt*/); // temporarily disabled crypt to see whether there's any performance gains
			setSize(size);
		}

		void TcpHeader::setBogus(std::uint32_t p)
		{
			bogus = p & 0xF;
		}

		void TcpHeader::setSessionId(std::uint32_t s)
		{
			sessionId = s & 0x3FFF;
		}

		void TcpHeader::setSize(std::uint32_t s)
		{
			size = s & 0x7FF;
		}

		void TcpHeader::setCrypt(std::uint32_t c)
		{
			crypt = c & 0x7;
		}

		std::uint32_t TcpHeader::getData() const
		{
			return (bogus | (sessionId << 4) | (size << 18) | (crypt << 29));
		}

		std::uint32_t TcpHeader::getBogus() const
		{
			return bogus;
		}

		std::uint32_t TcpHeader::getSessionId() const
		{
			return sessionId;
		}

		std::uint32_t TcpHeader::getSize() const
		{
			return size;
		}

		std::uint32_t TcpHeader::getCrypt() const
		{
			return crypt;
		}
	}
}