
#include <string>
#include <charconv>
#include "../../../include/Network/MainSession.h"
#include "../../../include/Network/MainSessionManager.h"
#include "../../../include/ChatCommands/SimpleCommands/SetCurrency.h"
#include "Enums/PlayerEnums.h"

#include "ChatCommands/ISimpleCommand.h"

namespace Main
{
	namespace Command
	{
		SetMaxCurrency::SetMaxCurrency(const Common::Enums::PlayerGrade requiredGrade)
			: ISimpleCommand{ requiredGrade }
		{
		}

		void SetMaxCurrency::execute(Main::Network::Session& session, Main::Network::SessionsManager& sessionManager, Common::Network::Packet& response)
		{
			this->m_confirmationMessage += "success"; // Additional arguments are ignored, e.g. /setcurrency 5

			response.setOrder(307);
			struct Message
			{
				std::uint32_t rt{};
				std::uint32_t mp{};
			};
			session.setAccountMicroPoints(536870912);
			session.setAccountRockTotens(536870912);

			const auto& accountInfo = session.getAccountInfo();
			Message message{ accountInfo.rockTotens, accountInfo.microPoints };
			response.setData(reinterpret_cast<std::uint8_t*>(&message), sizeof(message));
			session.asyncWrite(response);

			sendConfirmation(response, session);
		}

		void SetMaxCurrency::getCommandUsage(Main::Network::Session& session, Common::Network::Packet& response)
		{
			this->m_confirmationMessage += "/setcurrency: sets rt, mp and coins to their maximum values";
			sendConfirmation(response, session);
		}
	};
}