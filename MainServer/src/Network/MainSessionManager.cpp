

#include <unordered_map>
#include <functional>
#include "../../include/Network/MainSessionManager.h"
#include "../../include/Handlers/Room/RoomLeaveHandler.h"
#include <boost/interprocess/shared_memory_object.hpp> 
#include <boost/interprocess/mapped_region.hpp> 
#include <Utils/IPC_Structs.h>

namespace Main
{
	namespace Network
	{
		void SessionsManager::addSession(Main::Network::Session* session)
		{
			m_sessionsBySessionId.emplace(session->getId(), session);
			m_sessionsByAccountId.emplace(session->getAccountInfo().accountID, session);
		}

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
				if (m_sessionsBySessionId[sessionId]->getRoomNumber())
				{
					auto roomOpt = roomsManager->getRoomByNumber(m_sessionsBySessionId[sessionId]->getRoomNumber());
					if (roomOpt == std::nullopt) return;
					auto& room = roomOpt.value().get();

					if (m_sessionsBySessionId[sessionId]->isInMatch())
					{
						// if host, remove host from both match + room. removeHostFromMatch does that.
						if (room.isHost(m_sessionsBySessionId[sessionId]->getAccountInfo().uniqueId))
						{
							// This will also remove the host from the Room + notify the room that the host left the room
							bool roomMustBeClosed = room.removeHostFromMatch(); // Also notifies cast whether the room must be broken
							if (roomMustBeClosed)
							{
								roomsManager->removeRoom(room.getRoomNumber());
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
							room.broadcastToRoom(response);
							m_sessionsBySessionId[sessionId]->setIsInMatch(false);

							room.removePlayer(m_sessionsBySessionId[sessionId], 1);

							// unnedeed, since if this is the case, client just sends endMatch request.
							//room.removeHostIfAloneAndModeDoesntAllowIt();
						}
					}
					else
					{
						bool roomMustBeClosed = room.removePlayer(m_sessionsBySessionId[sessionId], 1); // 1 = extra = leave room normally
						// nb. removePlayer does NOT notify cast that the room must be removed.
						if (roomMustBeClosed)
						{
							roomsManager->removeRoom(room.getRoomNumber());
						}
					}
					m_sessionsBySessionId[sessionId]->leaveRoom();
				}
				m_sessionsBySessionId[sessionId]->clear();
				m_sessionsBySessionId.erase(sessionId);
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
			for (auto& [sessionId, otherSession] : m_sessionsBySessionId)
			{
				otherSession->asyncWrite(message);
			}
		}

		void SessionsManager::broadcastExceptSelf(std::size_t selfSessionId, const Common::Network::Packet& message) const
		{
			for (auto& [sessionId, otherSession] : m_sessionsBySessionId)
			{
				if (selfSessionId == sessionId) continue; // don't broad cast to sender
				otherSession->asyncWrite(message);
			}
		}

		void SessionsManager::broadcastToLobbyExceptSelf(std::size_t selfSessionId, const Common::Network::Packet& message) const
		{
			for (auto& [sessionId, otherSession] : m_sessionsBySessionId)
			{
				if (selfSessionId == sessionId || !otherSession->isInLobby()) continue; // don't broad cast to sender
				otherSession->asyncWrite(message);
			}
		}

		void SessionsManager::broadcastToClan(std::uint64_t selfSessionId, const Common::Network::Packet& message) const
		{
			const auto& selfAccountInfo = m_sessionsBySessionId.at(selfSessionId)->getAccountInfo();
			for (auto& [sessionId, otherSession] : m_sessionsBySessionId)
			{
				if (selfSessionId == sessionId) continue; // don't broad cast to sender
				const auto& otherAccountInfo = otherSession->getAccountInfo();
				if (otherAccountInfo.clanId == selfAccountInfo.clanId) otherSession->asyncWrite(message);
			}
		}

		Main::Network::Session* SessionsManager::findSessionByName(const char* nickname)
		{
			for (auto& [sessionId, session] : m_sessionsBySessionId)
			{
				if (std::strcmp(session->getPlayerName(), nickname) == 0)
				{
					return session;
				}
			}
			return nullptr;
		}

		Main::Network::Session* SessionsManager::getSessionByAccountId(std::uint32_t aid)
		{
			for (const auto& currentSession : m_sessionsBySessionId)
			{
				if (currentSession.second->getAccountInfo().accountID == aid)
				{
					return currentSession.second;
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

		bool SessionsManager::sendTo(std::size_t sessionId, const Common::Network::Packet& packet)
		{
			if (m_sessionsBySessionId.contains(sessionId))
			{
				m_sessionsBySessionId[sessionId]->asyncWrite(packet);
				return true;
			}
			return false;
		}
	};
}
