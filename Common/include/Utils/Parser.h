
#ifndef PARSER_HEADER_H
#define PARSER_HEADER_H

#include <cstdint>
#include <string>

#include "../Cryptography/Crypt.h"
#include "../Protocol/TcpHeader.h"
#include "../Protocol/CommandHeader.h"

namespace Common
{
	namespace Parser
	{
		void printTcpHeader(Common::Protocol::TcpHeader header);

		void printCommandHeader(Common::Protocol::CommandHeader command);

		void parse_cast(std::uint8_t* data, std::size_t len, std::size_t port, const std::string& origin,
			const std::string& to);

		void parse(std::uint8_t* data, std::size_t len, std::size_t port, const std::string& origin,
			const std::string& to, std::int32_t cryptKey, bool first = false);
	}
}

#endif
