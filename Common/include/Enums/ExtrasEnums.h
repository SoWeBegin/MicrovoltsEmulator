#ifndef ENUMS_COMMON_EXTRAS_H
#define ENUMS_COMMON_EXTRAS_H

namespace Common
{
	namespace Enums
	{
		enum ConnectionPacketExtra
		{
			MAIN_SUCCESS = 0,
			AUTH_SUCCESS = 34,
			AUTH_MAINTENANCE = 35,
			// can be any number except the ones listed in this enum
			AUTH_CONN_FAIL = 40,
			CAST_SUCCESS = 54
		};
	}
}

#endif