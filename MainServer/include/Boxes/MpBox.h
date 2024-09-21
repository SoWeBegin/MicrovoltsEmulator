#ifndef MICROPOINTS_BOX_H
#define MICROPOINTS_BOX_H

#include <cstdint>
#include <unordered_map>
#include "../MainEnums.h"
#include "BoxBase.h"
#include <random>

namespace Main
{
	namespace Box
	{
		class MpBox : public IBox
		{
		private:
			std::uint32_t m_mpValue;

		public:
			explicit MpBox(std::uint32_t mpValue)
				: m_mpValue{ mpValue }
			{
			}

			// returns the value of the MP box 
			std::uint32_t get() override
			{
				return m_mpValue;
			}
		};
	}
}
#endif