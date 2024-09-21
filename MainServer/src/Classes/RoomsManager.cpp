
#include <functional>
#include "../../include/Classes/Room.h"
#include "../../include/Classes/RoomsManager.h"
#include "../../include/Structures/Room/RoomsList.h"

namespace Main
{
	namespace Classes
	{
		void RoomsManager::addRoom(const Main::Classes::Room& room)
		{
			m_roomByNumber.emplace(room.getRoomNumber(), room);
		}

		void RoomsManager::removeRoom(std::uint16_t roomNum) 
		{
			if (m_roomByNumber.contains(roomNum)) 
			{
				m_roomByNumber[roomNum].removeAllPlayers();
				m_roomByNumber.erase(roomNum);
			}
		}

		std::size_t RoomsManager::getTotalRooms() const
		{
			return m_roomByNumber.size();
		}

		std::vector<Main::Structures::SingleRoom> RoomsManager::getRoomsList() const
		{
			std::vector<Main::Structures::SingleRoom> roomsList;
			for (const auto& [roomNumber, room] : m_roomByNumber)
			{
				roomsList.push_back(room.getRoomInfo());
			}
			return roomsList;
		}

		std::optional<std::reference_wrapper<Main::Classes::Room>> RoomsManager::getRoomByNumber(std::uint16_t roomNumber)
		{
			if (m_roomByNumber.contains(roomNumber))
			{
				return std::ref(m_roomByNumber[roomNumber]);
			}
			return std::nullopt;
		}

		void RoomsManager::broadcastToRoom(std::uint16_t roomNumber, Common::Network::Packet& packet)
		{
			if (!m_roomByNumber.contains(roomNumber)) return;
			auto& room = m_roomByNumber[roomNumber];
			room.broadcastToRoom(packet);
		}

		void RoomsManager::broadcastToRoomExceptSelf(std::uint16_t roomNumber, const Main::Structures::UniqueId& selfUniqueId, Common::Network::Packet& packet)
		{
			if (!m_roomByNumber.contains(roomNumber)) return;
			auto& room = m_roomByNumber[roomNumber];
			room.broadcastToRoomExceptSelf(packet, selfUniqueId);
		}

		void RoomsManager::broadcastToMatchExceptSelf(std::uint16_t roomNumber, const Main::Structures::UniqueId& selfUniqueId, Common::Network::Packet& packet, std::uint32_t extra)
		{
			if (!m_roomByNumber.contains(roomNumber)) return;
			auto& room = m_roomByNumber[roomNumber];
			room.broadcastToMatchExceptSelf(packet, selfUniqueId, extra);
		}

		void RoomsManager::broadcastOutsideMatchExceptSelf(std::uint16_t roomNumber, const Main::Structures::UniqueId& selfUniqueId, Common::Network::Packet& packet, std::uint32_t extra)
		{
			if (!m_roomByNumber.contains(roomNumber)) return;
			auto& room = m_roomByNumber[roomNumber];
			room.broadcastOutsideMatchExceptSelf(packet, selfUniqueId, extra);
		}
	};
}
