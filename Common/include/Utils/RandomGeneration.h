#ifndef COMMON_RANDOM_GENERATION_H
#define COMMON_RANDOM_GENERATION_H

#include <ctime>
#include <cstdint>
#include <cstdlib>
#include <random>

namespace Common
{
	namespace Random
	{
		inline std::uint32_t generateAccountKey()
		{
			static std::mt19937 gen(std::random_device{}());

			// for some reaso std::numeric_limits<std::uint32_t>::max() gives compiler errors
			// apparently it has to do with Parser.cpp, if we move everything inside Parser.h then ::max() works fine...
			std::uniform_int_distribution<std::uint32_t> distribution(1, static_cast<std::uint32_t>(-1)); 

			return distribution(gen);
		}

		inline std::uint32_t generateRandomBogus()
		{
			return std::rand() % 16;
		}

		inline std::uint16_t generateRandomMission()
		{
			return std::rand() % 4;
		}

		inline std::uint32_t generateRandomOption()
		{
			return std::rand() % 256;
		}

		inline std::uint32_t generateRandomExtra()
		{
			return generateRandomOption();
		}
	}
}

#endif