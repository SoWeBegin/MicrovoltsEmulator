
#include <cstdint>
#include <unordered_map>
#include <utility>
#include "../../include/Network/CastSession.h"
#include "../../include/Classes/Room.h"
#include <Utils/Logger.h>
#include <Utils/IPC_Structs.h>
#include <Enums/RoomEnums.h>

namespace Cast
{
	namespace Classes
	{
		Room::Room(std::uint64_t hostSessionId, Cast::Network::Session* hostSession)
		{
			m_players.insert(std::pair{hostSessionId, hostSession});
			hostSession->setRoomId(hostSessionId);
			m_roomNumber = ++idCounter;
			hostSession->setRoomNumber(m_roomNumber);

			auto& logger = ::Utils::Logger::getInstance();
			logger.log("Creating Room. Added Host (ID: " + std::to_string(hostSessionId)
				+ ", RoomNumber: " + std::to_string(hostSession->getRoomNumber()) + ")", ::Utils::LogType::Info, "Room::Room");
		}

		std::uint32_t Room::getRoomNumber() const
		{
			return m_roomNumber;
		}

		std::size_t Room::getTotalPlayersInMatch() const
		{
			std::size_t total = 0;
			for (const auto& current : m_players)
			{
				if (current.second->isInMatch())
				{
					++total;
				}
			}
			return total;
		}

		void Room::endMatch()
		{
			for (const auto& current : m_players)
			{
				if (current.second->isInMatch())
				{
					current.second->setIsInMatch(false);
				}
			}
		}

		std::size_t Room::getTotalPlayers() const
		{
			return m_players.size();
		}

		void Room::removeAllPlayers()
		{
			for (const auto& currentPlayer : m_players)
			{
				removePlayer(currentPlayer.second->getId());
			}
		}

		// Returns true if the room must be closed because there are no players left, false otherwise
		bool Room::removePlayer(std::uint64_t playerSessionId)
		{
			if (!m_players.contains(playerSessionId))
			{
				std::cout << "Room::removePlayer: player not found\n";
			}
			else
			{
				std::cout << "removing player from room: " << m_players[playerSessionId]->getRoomNumber() << "\n";
				m_players[playerSessionId]->setIsInMatch(false);
				m_players[playerSessionId]->setRoomNumber(-1);
				m_players.erase(playerSessionId);
			}
			return m_players.empty();
		}

		bool Room::containsPlayer(std::uint64_t playerSessionId) const
		{
			return m_players.contains(playerSessionId);
		}

		void Room::addPlayer(Cast::Network::Session* playerSession)
		{
			auto& logger = ::Utils::Logger::getInstance(true);
			logger.log("Added playerID: " + std::to_string(playerSession->getId())
			+ " to room number: " + std::to_string(m_roomNumber), ::Utils::LogType::Info, "Room::addPlayer");
			
			// Check whether m_players or m_observerPlayers is full?
			m_players[playerSession->getId()] = playerSession;
			playerSession->setRoomNumber(m_roomNumber);
		}

		void Room::broadcastToRoomExceptSelf(std::uint64_t selfSessionId, Common::Network::Packet& packet)
		{
			for (const auto& [sessionId, session] : m_players)
			{
				if (selfSessionId == sessionId) continue;
				packet.setTcpHeader(sessionId, Common::Enums::NO_ENCRYPTION);
				session->asyncWrite(packet);
			}
		}

		void Room::broadcastToRoom(Common::Network::Packet& packet)
		{
			for (const auto& [sessionId, session] : m_players)
			{
				packet.setTcpHeader(sessionId, Common::Enums::NO_ENCRYPTION);
				session->asyncWrite(packet);
			}
		}

		bool Room::isInMatch(std::uint64_t playerSessionId) const
		{
			for (const auto& current : m_players)
			{
				if (current.second->getId() == playerSessionId && current.second->isInMatch())
				{
					return true;
				}
			}
			return false;
		}


		void Room::playerForwardToHost(std::uint64_t hostSessionId, std::uint64_t senderSessionId, Common::Network::Packet& packet)
		{
			auto& logger = ::Utils::Logger::getInstance();

			packet.setTcpHeader(senderSessionId, Common::Enums::NO_ENCRYPTION);

			if (m_players.contains(hostSessionId))
			{
				m_players[hostSessionId]->asyncWrite(packet);

				logger.log("Forwarding Packet from SenderSessionID: " + std::to_string(senderSessionId) +
					" to Room Host SessionID: " + std::to_string(hostSessionId), ::Utils::LogType::Info, "Room::playerForwardToHost");

			}
		}

		void Room::hostForwardToPlayer(std::uint64_t hostSessionId, std::uint64_t playerId, Common::Network::Packet& packet, bool useHostIdInTcpHeader)
		{
			auto& logger = ::Utils::Logger::getInstance();

			if (useHostIdInTcpHeader)
			{
				packet.setTcpHeader(hostSessionId, Common::Enums::NO_ENCRYPTION);
			}
			else
			{
				packet.setTcpHeader(playerId, Common::Enums::NO_ENCRYPTION);
			}
			if (m_players.contains(playerId))
			{
				m_players[playerId]->asyncWrite(packet);

				logger.log("Forwarding Packet from Host: " + std::to_string(hostSessionId) +
					" to Player SessionID: " + std::to_string(playerId), ::Utils::LogType::Info, "Room::hostForwardToPlayer");
			}
		}

		void Room::setMap(std::uint32_t mapId)
		{
			m_map = mapId;
		}

		void Room::setMode(std::uint32_t modeId)
		{
			m_mode = modeId;
		}

		std::uint32_t Room::getMap() const
		{
			return m_map;
		}

		std::uint32_t Room::getMode() const
		{
			return m_mode;
		}

		void Room::leaveAllPLayers()
		{
			std::uint32_t totalInMatch = 0;
			// Count total players in match.
			for (auto& player : m_players)
			{
				if (player.second->isInMatch())
				{
					++totalInMatch;
				}
			}

			if (totalInMatch == 1)
			{
				if (m_mode == Common::Enums::SquareMode || m_mode == Common::Enums::AiBattle || m_mode == Common::Enums::BossBattle)
				{
					return;
				}
				for (auto& player : m_players)
				{
					player.second->setIsInMatch(false);
				}
			}
		}

		void Room::setTick(std::uint32_t tick)
		{
			m_tick = tick;
			std::cout << "tick set to : " << tick << '\n';
		}

		std::uint32_t Room::getTick() const
		{
			return m_tick;
		}

	};
}