#ifndef ROOMS_MANAGER_H
#define ROOMS_MANAGER_H

#include <unordered_map>
#include <functional>
#include "Room.h"
#include "../Structures/Room/RoomsList.h"

namespace Main
{
	namespace Classes
	{
		class RoomsManager
		{
		private:
			std::unordered_map<std::uint16_t, Main::Classes::Room> m_roomByNumber{};

		public:
			void addRoom(const Main::Classes::Room& room);

			void removeRoom(std::uint16_t roomNum);

			std::size_t getTotalRooms() const;

			std::vector<Main::Structures::SingleRoom> getRoomsList() const;

			std::optional<std::reference_wrapper<Main::Classes::Room>> getRoomByNumber(std::uint16_t roomNumber);

			void broadcastToRoom(std::uint16_t roomNumber, Common::Network::Packet& packet);

			void broadcastToRoomExceptSelf(std::uint16_t roomNumber, const Main::Structures::UniqueId& selfUniqueId, Common::Network::Packet& packet);

			// For in-room messages
			void broadcastToMatchExceptSelf(std::uint16_t roomNumber, const Main::Structures::UniqueId& selfUniqueId, Common::Network::Packet& packet, std::uint32_t extra);

			void broadcastOutsideMatchExceptSelf(std::uint16_t roomNumber, const Main::Structures::UniqueId& selfUniqueId, Common::Network::Packet& packet, std::uint32_t extra);
		};
	}
}
#endif