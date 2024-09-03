#ifndef COMMON_AONEARG_NONNUMERICAL_DBCOMMAND_HEADER
#define COMMON_AONEARG_NONNUMERICAL_DBCOMMAND_HEADER

#include <string>
#include "../../Network/Packet.h"
#include "IComplexDbCommand.h"

namespace Common
{
	namespace Command
	{
		template<typename SessionType, typename DatabaseType>
		class AbstractOneArgNonNumericalDbCommand : public IComplexDbCommand<SessionType, DatabaseType>
		{
		protected:
			std::string m_value{};

		public:
			AbstractOneArgNonNumericalDbCommand(Common::Enums::PlayerGrade requiredGrade)
				: IComplexDbCommand<SessionType, DatabaseType>{ "^(.*)\\s+(.*)$", requiredGrade }
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

			virtual ~AbstractOneArgNonNumericalDbCommand() = default;
			// subclasses must override getCommandUsage()
		};
	}
}
#endif