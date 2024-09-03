#ifndef COMMON_ISIMPLE_ROOM_COMMAND_HEADER
#define COMMON_ISIMPLE_ROOM_COMMAND_HEADER

#include <string>
#include "../Network/Packet.h"
#include "AbstractCommandBase.h"

namespace Common
{
	namespace Command
	{
		template<typename SessionType, typename RoomsManager>
		class ISimpleRoomCommand : public AbstractCommandBase<SessionType, RoomsManager>
		{
		public:
			ISimpleRoomCommand(const Common::Enums::PlayerGrade requiredGrade)
				: AbstractCommandBase<SessionType, RoomsManager>{ requiredGrade }
			{
			}

			virtual ~ISimpleRoomCommand() = default;
			virtual void execute(SessionType& session, RoomsManager& room, std::size_t roomNumber, Common::Network::Packet& response) = 0;
		};
	}
}
#endif