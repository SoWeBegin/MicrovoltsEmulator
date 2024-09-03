
#include <unordered_map>
#include <functional>
#include "../../include/Network/CastSession.h"
#include "../../include/Network/SessionsManager.h"
#include <vector>
#include "../../../MainServer/include/Structures/AccountInfo/MainAccountInfo.h"
#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_upgradable_mutex.hpp>
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
			auto& logger = ::Utils::Logger::getInstance(true, false, "cast_log.txt");
			logger.log<::Utils::LogDestination::File>("Trying to add session with ID: " + std::to_string(sessionId), ::Utils::LogType::Info, "SessionsManager::addSession");

			if (!m_sessionsBySessionId.contains(sessionId))
			{
				m_sessionsBySessionId.emplace(sessionId, session);
				session->setSessionId(sessionId);

				logger.log<::Utils::LogDestination::File>("Added  session with ID: " + std::to_string(sessionId), ::Utils::LogType::Info, "SessionsManager::addSession");
			}
		}

		void SessionsManager::removeSession(std::size_t sessionId)
		{
			auto& logger = ::Utils::Logger::getInstance(true);
			logger.log("Trying to remove SessionID: " + std::to_string(sessionId), ::Utils::LogType::Info, "SessionsManager::removeSession");

			// The player is inside a room.
			if (m_sessionsBySessionId.contains(sessionId) && m_sessionsBySessionId[sessionId]->getRoomNumber() != -1)
			{
			}
			if (m_sessionsBySessionId.contains(sessionId))
			{
				m_sessionsBySessionId.erase(sessionId);
				logger.log<::Utils::LogDestination::File>("Session removed from sessions list: " + std::to_string(sessionId), ::Utils::LogType::Info, "SessionsManager::removeSession");
			}
		}
	};
}