

#include <unordered_map>
#include <functional>
#include "../../include/Network/MainSessionManager.h"
#include "../../include/Handlers/Room/RoomLeaveHandler.h"

namespace Main
{
	namespace Network
	{
		void SessionsManager::addSession(Main::Network::Session* session)
		{
			m_sessionsBySessionId.emplace(session->getId(), session);
			m_sessionsVector.push_back(session);
		}

		// Todo: This must update "IsOnline" to 0!
		void SessionsManager::removeSession(std::size_t sessionId)
		{
			if (m_sessionsBySessionId.contains(sessionId))
			{
				// Notify logout to online friends
				for (auto& targetFriendSession : m_sessionsBySessionId[sessionId]->getFriendSessions())
				{
					if (!targetFriendSession.second) continue;
					targetFriendSession.second->logFriend(0, m_sessionsBySessionId[sessionId]->getAccountInfo().accountID);
					targetFriendSession.second->updateFriendSession(m_sessionsBySessionId[sessionId], true);
				}
				// Reset room state
				if (Main::Classes::Room* room = roomsManager->getRoomByNumber(m_sessionsBySessionId[sessionId]->getRoomNumber()))
				{
					if (m_sessionsBySessionId[sessionId]->isInMatch())
					{
						// if host, remove host from both match + room. removeHostFromMatch does that.
						if (room->isHost(m_sessionsBySessionId[sessionId]->getAccountInfo().uniqueId))
						{
							// This will also remove the host from the Room + notify the room that the host left the room
							bool roomMustBeClosed = room->removeHostFromMatch(); // Also notifies cast whether the room must be broken
							if (roomMustBeClosed)
							{
								roomsManager->removeRoom(room->getRoomNumber());
							}
						}
						else
						{
							// Remove the player from match + room.
							// Notify room that this non host left the match.
							Common::Network::Packet response;
							response.setTcpHeader(sessionId, Common::Enums::USER_LARGE_ENCRYPTION);
							response.setOrder(256);
							response.setExtra(0);
							auto uniqueId = m_sessionsBySessionId[sessionId]->getAccountInfo().uniqueId;
							response.setData(reinterpret_cast<std::uint8_t*>(&uniqueId), sizeof(uniqueId));
							room->broadcastToRoom(response);
							m_sessionsBySessionId[sessionId]->setIsInMatch(false);

							room->removePlayer(m_sessionsBySessionId[sessionId], 1);
						}
					}
					else
					{
						bool roomMustBeClosed = room->removePlayer(m_sessionsBySessionId[sessionId], 1); // 1 = extra = leave room normally
						// nb. removePlayer does NOT notify cast that the room must be removed.
						if (roomMustBeClosed)
						{
							roomsManager->removeRoom(room->getRoomNumber());
						}
					}
				}
				m_sessionsBySessionId.erase(sessionId);

				auto it = std::find_if(m_sessionsVector.begin(), m_sessionsVector.end(),
					[&](Session* session) { return session->getSessionId() == sessionId; });
				if (it != m_sessionsVector.end())
				{
					m_sessionsVector.erase(it);
				}
			}
		}

		void SessionsManager::setRoomsManager(Main::Classes::RoomsManager* rmManager)
		{
			roomsManager = rmManager;
		}


		const std::unordered_map<std::uint64_t, Main::Network::Session*>& SessionsManager::getAllSessions() const
		{
			return m_sessionsBySessionId;
		}

		std::unordered_map<std::uint64_t, Main::Network::Session*>& SessionsManager::getAllSessions()
		{
			return m_sessionsBySessionId;
		}

		void SessionsManager::broadcast(const Common::Network::Packet& message) const
		{
			for (auto& currentSession : m_sessionsVector)
			{
				currentSession->asyncWrite(message);
			}
		}

		void SessionsManager::broadcastExceptSelf(std::size_t selfSessionId, const Common::Network::Packet& message) const
		{
			for (auto& currentSession : m_sessionsVector)
			{
				if (currentSession->getId() == selfSessionId) continue;
				currentSession->asyncWrite(message);
			}
		}

		void SessionsManager::broadcastToLobbyExceptSelf(std::size_t selfSessionId, const Common::Network::Packet& message) const
		{
			for (auto& currentSession : m_sessionsVector)
			{
				if (selfSessionId == currentSession->getId() || !currentSession->isInLobby()) continue; 
				currentSession->asyncWrite(message);
			}
		}

		void SessionsManager::broadcastToClan(std::uint64_t selfSessionId, const Common::Network::Packet& message) const
		{
			const auto& selfAccountInfo = m_sessionsBySessionId.at(selfSessionId)->getAccountInfo();
			for (auto& currentSession : m_sessionsVector)
			{
				if (selfSessionId == currentSession->getId()) continue;
				if (selfAccountInfo.clanId == currentSession->getAccountInfo().clanId)
				{
					currentSession->asyncWrite(message);
				}
			}
		}

		Main::Network::Session* SessionsManager::findSessionByName(const char* nickname)
		{
			for (auto& currentSession : m_sessionsVector)
			{
				if (std::strcmp(currentSession->getPlayerName(), nickname) == 0)
				{
					return currentSession;
				}
			}
			return nullptr;
		}

		Main::Network::Session* SessionsManager::getSessionByAccountId(std::uint32_t aid)
		{
			for (const auto& currentSession : m_sessionsVector)
			{
				if (currentSession->getAccountInfo().accountID == aid)
				{
					return currentSession;
				}
			}
			return nullptr;
		}

		Main::Network::Session* SessionsManager::getSessionBySessionId(std::size_t sessionId)
		{
			if (m_sessionsBySessionId.contains(sessionId))
			{
				return m_sessionsBySessionId[sessionId];
			}
			return nullptr;
		}

		std::uint32_t SessionsManager::getTotalSessions() const
		{
			return m_sessionsBySessionId.size();
		}

		bool SessionsManager::sendTo(std::size_t sessionId, const Common::Network::Packet& packet)
		{
			if (m_sessionsBySessionId.contains(sessionId))
			{
				m_sessionsBySessionId[sessionId]->asyncWrite(packet);
				return true;
			}
			return false;
		}

		Common::Network::Packet SessionsManager::prepareMessage(const std::string& message) const
		{
			Common::Network::Packet response;
			response.setOrder(316);
			response.setExtra(1);
			std::string m_confirmationMessage{ std::string(16, '0') };
			m_confirmationMessage += message;
			response.setData(reinterpret_cast<std::uint8_t*>(m_confirmationMessage.data()), m_confirmationMessage.size());
			return response;
		}

		void SessionsManager::broadcastMessage(const std::string& message) const
		{
			auto response = prepareMessage(message);
			for (auto& currentSession : m_sessionsVector)
			{
				currentSession->asyncWrite(response);
			}
		}

		void SessionsManager::broadcastMessageExceptSelf(std::size_t selfSessionId, const std::string& message) const
		{
			auto response = prepareMessage(message);
			for (auto& currentSession : m_sessionsVector)
			{
				if (currentSession->getId() == selfSessionId) continue;
				currentSession->asyncWrite(response);
			}
		}
	};
}
