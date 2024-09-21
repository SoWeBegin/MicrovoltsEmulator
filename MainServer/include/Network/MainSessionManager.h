#ifndef MAIN_SESSIONS_MANAGER_H
#define MAIN_SESSIONS_MANAGER_H

#include <unordered_map>
#include <vector>
#include <functional>
#include "MainSession.h"
#include "../Classes/RoomsManager.h"

namespace Main
{
	namespace Network
	{
		class SessionsManager
		{
		private:
			std::unordered_map<std::uint64_t, Main::Network::Session*> m_sessionsBySessionId{};
			std::vector<Main::Network::Session*> m_sessionsVector{};
			Main::Classes::RoomsManager* roomsManager;

		public:
			void setRoomsManager(Main::Classes::RoomsManager* roomsManager);

			void addSession(Main::Network::Session* session);

			void removeSession(std::size_t sessionId);

			const std::unordered_map<std::uint64_t, Main::Network::Session*>& getAllSessions() const;

			std::unordered_map<std::uint64_t, Main::Network::Session*>& getAllSessions();

			void broadcast(const Common::Network::Packet& message) const;

			void broadcastExceptSelf(std::size_t selfSessionId, const Common::Network::Packet& message) const;

			void broadcastToLobbyExceptSelf(std::size_t selfSessionId, const Common::Network::Packet& message) const;

			void broadcastToClan(std::uint64_t selfSessionId, const Common::Network::Packet& message) const;

			Main::Network::Session* findSessionByName(const char* nickname);

			Main::Network::Session* getSessionByAccountId(std::uint32_t aid);

			Main::Network::Session* getSessionBySessionId(std::size_t sessionId);

			std::uint32_t getTotalSessions() const;

			bool sendTo(std::size_t sessionId, const Common::Network::Packet& packet);

			Main::Classes::RoomsManager* getRoomsManager()
			{
				return roomsManager;
			}
		};
	}
}
#endif