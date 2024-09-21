#ifndef BOX_BASE_H
#define BOX_BASE_H

#include <cstdint>

namespace Main
{
	namespace Box
	{
		struct IBox
		{
			virtual std::uint32_t get() = 0;
		};
	}
}

#endif