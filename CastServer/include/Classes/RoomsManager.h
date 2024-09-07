#ifndef ROOMS_MANAGER_H
#define ROOMS_MANAGER_H

#include <unordered_map>
#include "Room.h"
#include "../include/Enums/RoomEnums.h"

namespace Cast
{
	namespace Classes
	{
		class RoomsManager
		{
		private:
			std::unordered_map<std::uint64_t, std::shared_ptr<Cast::Classes::Room>> m_playerSessionIdToRoom{};

		public:
			void addRoom(std::shared_ptr<Cast::Classes::Room> room, std::uint64_t playerId);
		
			// Debug
			void printRoomInfo(std::uint64_t sessionId, const std::string& furtherInfo)
			{
			}

			void switchRoomJoinOrExit(Cast::Network::Session& session, std::uint64_t hostSessionId = -1);

			void broadcastToRoomExceptSelf(std::uint64_t selfSessionId, Common::Network::Packet& packet);

			void broadcastToRoom(std::uint64_t hostSessionId, Common::Network::Packet& packet);

			void playerForwardToHost(std::uint64_t hostSessionId, std::uint64_t senderSessionId, Common::Network::Packet& packet);

			void hostForwardToPlayer(std::uint64_t hostSessionId, std::uint64_t receiverSessionId, Common::Network::Packet& packet, bool useHostSessionIdInTcpHeader = true);

			void setMapFor(std::uint64_t hostId, std::uint32_t map);

			void setModeFor(std::uint64_t hostId, std::uint32_t mode);

			void endMatch(std::uint64_t hostId);

			std::uint32_t getMapOf(std::uint64_t hostId);

			void removePlayerFromRoom(std::uint64_t sessionIdToRemoveFromRoom);

			void leaveAllPlayers(std::uint64_t hostId);
		};
	}
}
#endif
