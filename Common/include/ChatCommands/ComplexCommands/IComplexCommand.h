#ifndef COMMON_ICOMPLEX_COMMAND_HEADER
#define COMMON_ICOMPLEX_COMMAND_HEADER

#include <string>
#include <regex>
#include "../../Network/Packet.h"
#include "../AbstractCommandBase.h"

namespace Common
{
	namespace Command
	{
		template<typename SessionType, typename SessionManagerType>
		class IComplexCommand : public AbstractCommandBase<SessionType, SessionManagerType>
		{
		protected:
			std::regex m_pattern{};

		public:
			IComplexCommand(const std::string& regexPattern, const Common::Enums::PlayerGrade requiredGrade)
				: AbstractCommandBase<SessionType, SessionManagerType>{ requiredGrade }
				, m_pattern(regexPattern)
			{
			}

			virtual void execute(const std::string& providedCommand, SessionType& session, SessionManagerType& sessionManager, Common::Network::Packet& response) = 0;
			virtual ~IComplexCommand() = default;
			virtual bool parseCommand(const std::string& providedCommand) = 0;
		};
	}
}
#endif