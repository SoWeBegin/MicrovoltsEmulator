
#include <unordered_map>
#include <functional>
#include "../../include/Network/CastSession.h"
#include "../../include/Network/SessionsManager.h"
#include <vector>
#include "../../../MainServer/include/Structures/AccountInfo/MainAccountInfo.h"
#include <Utils/Logger.h>

namespace Cast
{
	namespace Network
	{
		void SessionsManager::setRoomsManager(Cast::Classes::RoomsManager* roomsManager)
		{
			m_roomsManager = roomsManager;
		}

		void SessionsManager::addSession(Cast::Network::Session* session, std::size_t sessionId)
		{
			if (!m_sessionsBySessionId.contains(sessionId))
			{
				m_sessionsBySessionId.emplace(sessionId, session);
				session->setSessionId(sessionId);
			}
		}

		void SessionsManager::removeSession(std::size_t sessionId)
		{
			// The player is inside a room.
			if (m_sessionsBySessionId.contains(sessionId) && m_sessionsBySessionId[sessionId]->getRoomNumber() != -1)
			{
			}
			if (m_sessionsBySessionId.contains(sessionId))
			{
				m_sessionsBySessionId.erase(sessionId);
			}
		}
	};
}