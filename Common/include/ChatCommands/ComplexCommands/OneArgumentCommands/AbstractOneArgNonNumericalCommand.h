#ifndef COMMON_AONEARG_NONNUMERICAL_COMMAND_HEADER
#define COMMON_AONEARG_NONNUMERICAL_COMMAND_HEADER

#include <string>
#include "../../../Network/Packet.h"
#include "../IComplexCommand.h"

namespace Common
{
	namespace Command
	{
		template<typename SessionType, typename SessionManagerType>
		class AbstractOneArgNonNumericalCommand : public IComplexCommand<SessionType, SessionManagerType>
		{
		protected:
			std::string m_value{};

		public:
			AbstractOneArgNonNumericalCommand(Common::Enums::PlayerGrade requiredGrade)
				: IComplexCommand<SessionType, SessionManagerType>{ "^(.*)\\s+(.*)$", requiredGrade}
			{
			}

			bool parseCommand(const std::string& providedCommand) override
			{
				std::smatch match;
				if (std::regex_match(providedCommand, match, this->m_pattern)) 
				{
					m_value = match[2].str();
					return true;
				}
				return false;
			}

			virtual ~AbstractOneArgNonNumericalCommand() = default;
			// subclasses must override getCommandUsage()
		};
	}
}
#endif