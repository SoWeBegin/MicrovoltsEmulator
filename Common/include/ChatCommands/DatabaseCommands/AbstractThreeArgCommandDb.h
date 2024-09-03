#ifndef COMMON_ATHREEARG_DB_COMMAND_HEADER
#define COMMON_ATHREEARG_DB_COMMAND_HEADER

#include <string>
#include "IComplexDbCommand.h"
#include <regex>

#include <iostream>

namespace Common
{
	namespace Command
	{
		template<typename SessionType, typename DatabaseType>
		class AbstractThreeArgCommandDb : public IComplexDbCommand<SessionType, DatabaseType> 
		{
		protected:
			std::string m_username{};
			std::string m_password{};
			std::string m_nickname{};

		public:
			AbstractThreeArgCommandDb(Common::Enums::PlayerGrade requiredGrade)
				: IComplexDbCommand<SessionType, DatabaseType>{ R"(^(\w+)\s+(\w+)\s+(\w+)\s+(\w+)$)", requiredGrade}
			{
			}

			bool parseCommand(const std::string& providedCommand) override
			{
				std::smatch match;
				if (std::regex_match(providedCommand, match, this->m_pattern))
				{
					m_username = match[2].str();
					m_password = match[3].str();
					m_nickname = match[4].str();
					return true;
				}
				return false;
			}

			virtual ~AbstractThreeArgCommandDb() = default;
			// subclasses must override getCommandUsage()
		};
	}
}
#endif
