#ifndef COMMON_AONEARG_NONNUMERICAL_ROOM_COMMAND_HEADER
#define COMMON_AONEARG_NONNUMERICAL_ROOM_COMMAND_HEADER

#include <string>
#include "../../../Network/Packet.h"
#include "../IComplexRoomCommand.h"

namespace Common
{
	namespace Command
	{
		template<typename SessionType, typename RoomsManagerType>
		class AbstractOneArgNonNumericalRoomCommand : public IComplexRoomCommand<SessionType, RoomsManagerType>
		{
		protected:
			std::string m_value{};

		public:
			AbstractOneArgNonNumericalRoomCommand(Common::Enums::PlayerGrade requiredGrade)
				: IComplexRoomCommand<SessionType, RoomsManagerType>{ "^([a-zA-Z]+)\\s([a-zA-Z]+)$", requiredGrade }
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

			virtual ~AbstractOneArgNonNumericalRoomCommand() = default;
			// subclasses must override getCommandUsage()
		};
	}
}
#endif