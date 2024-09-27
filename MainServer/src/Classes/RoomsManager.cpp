
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
			auto it = m_roomByNumber.find(roomNum);
			if (it == m_roomByNumber.end()) return;  
			it->second.removeAllPlayers();  
			m_roomByNumber.erase(it);    
		}

		std::size_t RoomsManager::getTotalRooms() const
		{
			return m_roomByNumber.size();
		}

		std::vector<Main::Structures::SingleRoom> RoomsManager::getRoomsList() const
		{
			std::vector<Main::Structures::SingleRoom> roomsList;
			roomsList.reserve(m_roomByNumber.size());
			for (const auto& [roomNumber, room] : m_roomByNumber)
			{
				roomsList.push_back(room.getRoomInfo());
			}
			return roomsList;
		}

		Main::Classes::Room* RoomsManager::getRoomByNumber(std::uint16_t roomNumber)
		{
			auto it = m_roomByNumber.find(roomNumber);
			if (it != m_roomByNumber.end())
			{
				return &it->second; 
			}
			return nullptr;
		}

		void RoomsManager::broadcastToRoom(std::uint16_t roomNumber, Common::Network::Packet& packet)
		{
			auto it = m_roomByNumber.find(roomNumber);
			if (it == m_roomByNumber.end()) return;  
			it->second.broadcastToRoom(packet);  
		}

		void RoomsManager::broadcastToRoomExceptSelf(std::uint16_t roomNumber, const Main::Structures::UniqueId& selfUniqueId, Common::Network::Packet& packet)
		{
			auto it = m_roomByNumber.find(roomNumber);
			if (it == m_roomByNumber.end()) return;  
			it->second.broadcastToRoomExceptSelf(packet, selfUniqueId);  
		}

		void RoomsManager::broadcastToMatchExceptSelf(std::uint16_t roomNumber, const Main::Structures::UniqueId& selfUniqueId, Common::Network::Packet& packet, std::uint32_t extra)
		{
			auto it = m_roomByNumber.find(roomNumber);
			if (it == m_roomByNumber.end()) return;  
			it->second.broadcastToMatchExceptSelf(packet, selfUniqueId, extra); 
		}

		void RoomsManager::broadcastOutsideMatchExceptSelf(std::uint16_t roomNumber, const Main::Structures::UniqueId& selfUniqueId, Common::Network::Packet& packet, std::uint32_t extra)
		{
			auto it = m_roomByNumber.find(roomNumber);
			if (it == m_roomByNumber.end()) return;  
			it->second.broadcastOutsideMatchExceptSelf(packet, selfUniqueId, extra);  
		}

	};
}
