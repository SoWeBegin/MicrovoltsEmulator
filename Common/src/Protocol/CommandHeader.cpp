#include "../../include/Protocol/CommandHeader.h"

namespace Common
{
	namespace Protocol
	{
		CommandHeader::CommandHeader(std::uint32_t data)
		{
			bogus = data & 0xF;
			mission = (data >> 4) & 0x3;
			order = (data >> 6) & 0x3FF;
			extra = (data >> 16) & 0xFF;
			option = (data >> 24) & 0xFF;
		}

		CommandHeader::CommandHeader(std::uint32_t mission, std::uint32_t order, std::uint32_t extra, std::uint32_t option,
			std::uint32_t bogus)
		{
			setBogus(bogus);
			setMission(mission);
			setOrder(order);
			setExtra(extra);
			setOption(option);
		}

		void CommandHeader::initialize(std::uint32_t order, std::uint32_t option, std::uint32_t extra, std::uint32_t mission)
		{
			setOrder(order);
			setOption(option);
			setExtra(extra);
			setMission(mission);
			setBogus(0);
		}

		void CommandHeader::setBogus(std::uint32_t p)
		{
			bogus = p & 0xF;
		}

		void CommandHeader::setMission(std::uint32_t m)
		{
			mission = m & 0x3;
		}

		void CommandHeader::setOrder(std::uint32_t p)
		{
			order = p & 0x3FF;
		}

		void CommandHeader::setExtra(std::uint32_t e)
		{
			extra = e & 0xFF;
		}

		void CommandHeader::setOption(std::uint32_t o)
		{
			option = o & 0xFF;
		}

		std::uint32_t CommandHeader::getData() const
		{
			return (bogus | (mission << 4) | (order << 6) | (extra << 16) | (option << 24));
		}

		std::uint32_t CommandHeader::getBogus() const
		{
			return bogus;
		}

		std::uint32_t CommandHeader::getMission() const
		{
			return mission;
		}

		std::uint32_t CommandHeader::getOrder() const
		{
			return order;
		}

		std::uint32_t CommandHeader::getExtra() const
		{
			return extra;
		}

		std::uint32_t CommandHeader::getOption() const
		{
			return option;
		}
	}
}