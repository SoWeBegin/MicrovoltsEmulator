
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
			m_playersVec.push_back(hostSession);
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
			for (const auto& current : m_playersVec)
			{
				if (current->isInMatch())
				{
					++total;
				}
			}
			return total;
		}

		void Room::endMatch()
		{
			for (auto& current : m_playersVec)
			{
				if (current->isInMatch())
				{
					current->setIsInMatch(false);
				}
			}
		}

		std::size_t Room::getTotalPlayers() const
		{
			return m_players.size();
		}

		void Room::removeAllPlayers()
		{
			for (auto& currentPlayer : m_players)
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

				auto it = std::remove_if(m_playersVec.begin(), m_playersVec.end(),
					[playerSessionId](const auto& player) 
					{
						return player->getId() == playerSessionId;
					});
				if (it != m_playersVec.end()) 
				{
					m_playersVec.erase(it, m_playersVec.end());
				}
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
			m_playersVec.push_back(playerSession);
			playerSession->setRoomNumber(m_roomNumber);
		}

		void Room::broadcastToRoomExceptSelf(std::uint64_t selfSessionId, Common::Network::Packet& packet)
		{
			for (auto& currentPlayer : m_playersVec)
			{
				auto currentSessionId = currentPlayer->getId();
				if (currentSessionId == selfSessionId) continue;
				packet.setTcpHeader(currentSessionId, Common::Enums::NO_ENCRYPTION);
				currentPlayer->asyncWrite(packet);
			}
		}

		void Room::broadcastToRoom(Common::Network::Packet& packet)
		{
			for (auto& currentPlayer : m_playersVec)
			{
				packet.setTcpHeader(currentPlayer->getId(), Common::Enums::NO_ENCRYPTION);
				currentPlayer->asyncWrite(packet);
			}
		}

		bool Room::isInMatch(std::uint64_t playerSessionId) const
		{
			auto it = m_players.find(playerSessionId);
			if (it != m_players.end() && it->second->isInMatch())
			{
				return true;
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
			for (auto& player : m_playersVec)
			{
				if (player->isInMatch())
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
				for (auto& player : m_playersVec)
				{
					player->setIsInMatch(false);
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
