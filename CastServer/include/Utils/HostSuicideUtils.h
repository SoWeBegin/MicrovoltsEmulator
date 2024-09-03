#ifndef HOST_SUICIDE_UTILS_H
#define HOST_SUICIDE_UTILS_H

#include <unordered_map>
#include "../include/Enums/RoomEnums.h"


namespace Cast
{
	namespace Utils
	{
		inline std::unordered_map<std::uint32_t, std::uint32_t> m_minHeightDeathPerMapLessThan{};
		inline std::unordered_map<std::uint32_t, std::uint32_t> m_minHeightDeathPerMapMoreThan{};

		inline void initHeightDeaths()
		{
			m_minHeightDeathPerMapLessThan[Common::Enums::Bitmap] = 20000; // less than = suicide
			m_minHeightDeathPerMapMoreThan[Common::Enums::Chess] = 50000;  // more than = suicide
			m_minHeightDeathPerMapMoreThan[Common::Enums::ToyFleet] = 54000; // more than = suicide
			m_minHeightDeathPerMapMoreThan[Common::Enums::TrackersFactory] = 50000; // more than = suicide
			m_minHeightDeathPerMapLessThan[Common::Enums::Beach] = 24000; // less than = suicide
			m_minHeightDeathPerMapMoreThan[Common::Enums::Neighboorhood] = 53000; // more than = suicide
			m_minHeightDeathPerMapLessThan[Common::Enums::RumpusRoom] = 22000; // less than = suicide
			m_minHeightDeathPerMapLessThan[Common::Enums::Cargo] = 20000; // less than = suicide
			m_minHeightDeathPerMapLessThan[Common::Enums::JunkYard] = 24300; // less than = suicide
			m_minHeightDeathPerMapLessThan[Common::Enums::Football] = 20000; // less than = suicide
			m_minHeightDeathPerMapLessThan[Common::Enums::Bitmap2] = 20000; // less than = suicide
			m_minHeightDeathPerMapMoreThan[Common::Enums::TheAftermath] = 57000; // more than = suicide
		}

		inline bool isSuicide(std::uint32_t map, std::uint32_t playerHeight)
		{
			if (m_minHeightDeathPerMapLessThan.contains(map))
			{
				return playerHeight <= m_minHeightDeathPerMapLessThan[map];
			}
			else if (m_minHeightDeathPerMapMoreThan.contains(map))
			{
				return playerHeight >= m_minHeightDeathPerMapMoreThan[map];
			}
			return false;
		}
	}
}

#endif