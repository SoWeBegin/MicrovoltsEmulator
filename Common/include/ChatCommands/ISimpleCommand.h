#ifndef COMMON_ISIMPLE_COMMAND_HEADER
#define COMMON_ISIMPLE_COMMAND_HEADER

#include <string>
#include "../Network/Packet.h"
#include "AbstractCommandBase.h"

namespace Common
{
	namespace Command
	{
		template<typename SessionType, typename SessionManagerType>
		class ISimpleCommand : public AbstractCommandBase<SessionType, SessionManagerType>
		{
		public:
			ISimpleCommand(const Common::Enums::PlayerGrade requiredGrade)
				: AbstractCommandBase<SessionType, SessionManagerType>{ requiredGrade }
			{
			}

			virtual ~ISimpleCommand() = default;
			virtual void execute(SessionType& session, SessionManagerType& sessionManager, Common::Network::Packet& response) = 0;
			// subclasses must override getCommandUsage() and execute()
		};
	}
}
#endif