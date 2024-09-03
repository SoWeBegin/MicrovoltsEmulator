#ifndef COMMON_MISCELLANEOUS_ENUMS_H
#define COMMON_MISCELLANEOUS_ENUMS_H

#include <cstdint>

namespace Common
{
	namespace Enums
	{
		enum ServerType
		{
			AUTH_SERVER,
			MAIN_SERVER,
			CAST_SERVER
		};

		enum EncryptionType : std::uint32_t
		{
			NO_ENCRYPTION = 0,
			DEFAULT_ENCRYPTION = 1,
			USER_ENCRYPTION = 2,
			DEFAULT_LARGE_ENCRYPTION = 3,
			USER_LARGE_ENCRYPTION = 4
		};
	}
}

#endif