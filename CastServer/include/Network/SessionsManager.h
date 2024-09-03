#ifndef	CAST_SESSIONS_MANAGER_H
#define CAST_SESSIONS_MANAGER_H

#include <unordered_map>
#include <functional>
#include "../Network/CastSession.h"
#include <vector>
#include "../../../MainServer/include/Structures/AccountInfo/MainAccountInfo.h"
#include "../Classes/RoomsManager.h"

namespace Cast
{
	namespace Network
	{
		class SessionsManager
		{
		private:
			std::unordered_map<std::uint64_t, Cast::Network::Session*> m_sessionsBySessionId{};
			Cast::Classes::RoomsManager* m_roomsManager;


		public:
			void setRoomsManager(Cast::Classes::RoomsManager* roomsManager);

			void addSession(Cast::Network::Session* session, std::size_t sessionId);

			void removeSession(std::size_t sessionId);
		};
	}
}
#endif