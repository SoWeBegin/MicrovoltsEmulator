#ifndef COMMON_AONEARGNUMERICAL_COMMAND_HEADER
#define COMMON_AONEARGNUMERICAL_COMMAND_HEADER

#include <string>
#include <charconv>
#include "../../../Network/Packet.h"
#include "../IComplexCommand.h"

#include <iostream>
namespace Common
{
	namespace Command
	{
		template<typename SessionType, typename SessionManagerType>
		class AbstractOneArgNumericalCommand : public IComplexCommand<SessionType, SessionManagerType>
		{
		protected:
			std::uint64_t m_value{};

		public:
			AbstractOneArgNumericalCommand(Common::Enums::PlayerGrade requiredGrade)
				: IComplexCommand<SessionType, SessionManagerType>{ "^([a-zA-Z]+)\\s(\\d+)$", requiredGrade}
			{
			}

			bool parseCommand(const std::string& providedCommand) override
			{
				std::cout << "Parse: OK1\n";
				std::smatch match;
				if (std::regex_match(providedCommand, match, this->m_pattern))
				{
					std::string numericalString = match[2].str();
					auto [ptr, ec] = std::from_chars(numericalString.c_str(), numericalString.c_str() + numericalString.size(), m_value);
					if (ec == std::errc())
					{
						return true;
					}
				}
				return false;
			}

			virtual ~AbstractOneArgNumericalCommand() = default;
			// subclasses must override getCommandUsage()
		};
	}
}
#endif