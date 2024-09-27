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

			void leaveAllPLayers();
		};
	}
}

#endif
