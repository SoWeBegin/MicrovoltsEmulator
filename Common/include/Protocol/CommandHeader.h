#ifndef COMMAND_HEADER_H
#define COMMAND_HEADER_H

#include <cstdint>
#include <cstdlib>
#include "../Utils/RandomGeneration.h"

namespace Common
{
	namespace Protocol
	{
#pragma pack(push, 1)
		class CommandHeader
		{
		private:
			std::uint32_t bogus : 4 = 0;    // Useless, probably for padding
			std::uint32_t mission : 2 = 0;  // More informations [max:4]
			std::uint32_t order : 10 = 0;   // Callback Number [max:1023]
			std::uint32_t extra : 8 = 0;    // Used to elaborate results [max:255]
			std::uint32_t option : 8 = 0;   // Options [max:255]
				
		public:
			constexpr CommandHeader() = default;

			explicit CommandHeader(std::uint32_t data);

			explicit CommandHeader(std::uint32_t mission, std::uint32_t order, std::uint32_t extra, std::uint32_t option,
				std::uint32_t bogus = 0);

			void initialize(std::uint32_t order, std::uint32_t option, std::uint32_t extra, std::uint32_t mission);

			void setBogus(std::uint32_t p);

			void setMission(std::uint32_t m);

			void setOrder(std::uint32_t p);

			void setExtra(std::uint32_t e);

			void setOption(std::uint32_t o);

			std::uint32_t getData() const;

			std::uint32_t getBogus() const;

			std::uint32_t getMission() const;

			std::uint32_t getOrder() const;

			std::uint32_t getExtra() const;

			std::uint32_t getOption() const;
		};
#pragma pack(pop)
	}
}

#endif