#ifndef COMMON_ICOMPLEX_DBCOMMAND_HEADER
#define COMMON_ICOMPLEX_DBCOMMAND_HEADER

#include <string>
#include <regex>
#include "../../Network/Packet.h"
#include "../AbstractCommandBase.h"

namespace Common
{
	namespace Command
	{
		template<typename SessionType, typename DatabaseManagerType>
		class IComplexDbCommand : public AbstractCommandBase<SessionType, DatabaseManagerType>
		{
		protected:
			std::regex m_pattern{};

		public:
			IComplexDbCommand(const std::string& regexPattern, const Common::Enums::PlayerGrade requiredGrade)
				: AbstractCommandBase<SessionType, DatabaseManagerType>{ requiredGrade }
				, m_pattern(regexPattern)
			{
			}

			virtual void execute(const std::string& providedCommand, SessionType& session, DatabaseManagerType& databaseManager, Common::Network::Packet& response) = 0;

			virtual ~IComplexDbCommand() = default;

			virtual bool parseCommand(const std::string& providedCommand) = 0;
		};
	}
}
#endif