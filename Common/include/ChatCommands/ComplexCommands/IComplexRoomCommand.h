#ifndef COMMON_ICOMPLEX_ROOM_COMMAND_HEADER
#define COMMON_ICOMPLEX_ROOM_COMMAND_HEADER

#include <string>
#include <regex>
#include "../../Network/Packet.h"
#include "../AbstractCommandBase.h"

namespace Common
{
	namespace Command
	{
		template<typename SessionType, typename RoomManagerType>
		class IComplexRoomCommand : public AbstractCommandBase<SessionType, RoomManagerType>
		{
		protected:
			std::regex m_pattern{};

		public:
			IComplexRoomCommand(const std::string& regexPattern, const Common::Enums::PlayerGrade requiredGrade)
				: AbstractCommandBase<SessionType, RoomManagerType>{ requiredGrade }
				, m_pattern(regexPattern)
			{
			}

			virtual void execute(const std::string& providedCommand, SessionType& session, RoomManagerType& roomsManager, Common::Network::Packet& response) = 0;
			virtual ~IComplexRoomCommand() = default;
			virtual bool parseCommand(const std::string& providedCommand) = 0;
		};
	}
}
#endif