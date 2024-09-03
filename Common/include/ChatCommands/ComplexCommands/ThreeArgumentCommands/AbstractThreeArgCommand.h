#ifndef COMMON_ATHREEARG_COMMAND_HEADER
#define COMMON_ATHREEARG_COMMAND_HEADER

#include <string>
#include "../../../Network/Packet.h"
#include "../IComplexCommand.h"
#include <charconv>
#include <regex>

#include <iostream>

namespace Common
{
	namespace Command
	{
		template<typename SessionType, typename SessionManagerType>
		class AbstractThreeArgCommand : public IComplexCommand<SessionType, SessionManagerType>
		{
		protected:
			std::string m_playername{};
			std::uint64_t m_duration{};
			std::string m_furtherInfo{};

		public:
			AbstractThreeArgCommand(Common::Enums::PlayerGrade requiredGrade)
				: IComplexCommand<SessionType, SessionManagerType>{ "^([a-zA-Z]+)\\s(.+)\\s(\\d+)\\s(.+)$", requiredGrade}
			{
			}

			bool parseCommand(const std::string& providedCommand) override
			{
				std::smatch match;
				if (std::regex_match(providedCommand, match, this->m_pattern))
				{
					m_playername = match[2].str();
					std::string numericalDuration = match[3].str();
					m_furtherInfo = match[4].str();
					auto [ptr, ec] = std::from_chars(numericalDuration.c_str(), numericalDuration.c_str() + numericalDuration.size(), m_duration);
					if (ec == std::errc())
					{
						return true;
					}
				}
				return false;
			}

			virtual ~AbstractThreeArgCommand() = default;
			// subclasses must override getCommandUsage()
		};
	}
}
#endif