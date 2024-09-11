#ifndef CAST_ROOM_H
#define CAST_ROOM_H

#include <cstdint>
#include <unordered_map>
#include <utility>
#include "../Network/CastSession.h"

namespace Cast
{
	namespace Classes
	{
		class Room
		{
		private:
			std::unordered_map<std::uint64_t, Cast::Network::Session*> m_players{};
			std::vector<Cast::Network::Session*> m_playersVec{};
			std::uint32_t m_map{};
			std::uint32_t m_mode{};
			std::uint32_t m_tick{};

			static inline std::uint32_t idCounter = 0;
			std::uint32_t m_roomNumber = -1;

		public:
			Room() = default;

			~Room()
			{
				// Just for debug.
				std::cout << "Room with number: " << m_roomNumber << " is being closed!\n";
			}

			// Debug
			void printRoomState(const std::string& furtherInfo) const
			{
				/*
				std::cout << "____________________________\n";
				std::cout << furtherInfo << ":\n\n";
				std::cout << "Room Information:\n";
				std::cout << "  Room Number: " << m_roomNumber << '\n';
				std::cout << "Players:\n";
				for (const auto& currentPlayer : m_players)
				{
					std::cout << "  - SessionID: " << currentPlayer.first
						//<< "\n    RoomID: " << currentPlayer.second->getRoomId()
						<< "\n    RoomNum: " << currentPlayer.second->getRoomNumber()
						<< "\n    Is In Match: " << (currentPlayer.second->isInMatch() ? "Yes" : "No") << '\n';
				}
				std::cout << "____________________________\n";
				*/
			}


			Room(std::uint64_t hostSessionId, Cast::Network::Session* hostSession);

			std::uint32_t getRoomNumber() const;

			void endMatch();

			const auto& getPlayers() { return m_players; }

			std::size_t getTotalPlayersInMatch() const;

			bool isInMatch(std::uint64_t playerSessionId) const;

			void removeAllPlayers();

			bool removePlayer(std::uint64_t playerSessionId);

			std::size_t getTotalPlayers() const;

			bool containsPlayer(std::uint64_t playerSessionId) const;

			void addPlayer(Cast::Network::Session* playerSession);

			void broadcastToRoomExceptSelf(std::uint64_t selfSessionId, Common::Network::Packet& packet);

			void broadcastToRoom(Common::Network::Packet& packet);

			void playerForwardToHost(std::uint64_t hostSessionId, std::uint64_t senderSessionId, Common::Network::Packet& packet);

			void hostForwardToPlayer(std::uint64_t, std::uint64_t playerId, Common::Network::Packet& packet, bool useHostIdInTcpHeader = true);
			
			void setMap(std::uint32_t mapId);

			void setMode(std::uint32_t modeId);

			std::uint32_t getMap() const;

			std::uint32_t getMode() const;

			std::uint32_t getTick() const;
			
			void setTick(std::uint32_t tick);

			void leaveAllPLayers();
		};
	}
}

#endif
