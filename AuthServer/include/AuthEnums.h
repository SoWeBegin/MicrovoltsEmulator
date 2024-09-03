#ifndef AUTH_ENUMERATIVES_H
#define AUTH_ENUMERATIVES_H

#include <cstdint>

namespace Auth
{
	namespace Enums
	{
		enum Login : std::uint32_t
		{
			INCORRECT = 0,           // Incorrect login id or password entered
			SUCCESS = 1,             // Confirmed account 
			SUCCESS2 = 37,           // Confirmed account
			DATA_ERROR = 4,			 // Nickname contains illegal characters because I can not log in, contact support
			ACCOUNT_BUSY = 5,        // Connection has timed out, would you like to reconnect?
			UNEXISTENT = 13,         // Incorrect login id or password entered
			TIME_EXPIRED = 24,		 // Connection failed, please choose another server or try again later
			SUSPENDED = 42,          // Account banned
			SHUTDOWN = 80            // No message at all, tilts the client (need to close and reopen)
		};

		enum ChannelStatus : std::uint32_t
		{
			LOW_TRAFFIC = 0,			  // [ChannelStatus: 0000000000]
			MEDIUM_TRAFFIC = 0x55555500,  // [ChannelStatus: 1111111111]
			HIGH_TRAFFIC = 0xAAAAAA00,    // [ChannelStatus: 2222222222]
			OFFLINE = 0xFFFFFF00		  // [ChannelStatus: 3333333333]
			// Cannot have more statuses, since any value & 3 <= 3
		};
	}
}

#endif