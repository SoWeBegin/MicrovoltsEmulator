#ifndef AUTHORIZATION_HANDLER_H
#define AUTHORIZATION_HANDLER_H

#include "../../Network/MainSession.h"
#include "../../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "Network/Packet.h"

namespace Main
{
	namespace Handlers
	{
		enum AuthorizationExtra
		{
			SUCCESS = 1,
			ACCOUNT_NOT_FOUND = 6,
			ID_AUTHORIZE_FULL = 7, // seemingly unused by the client
			INCORRECT_CLIENT_VERSION = 16, // seemingly unused by the client
			AUTHORIZATION_FAILED = 35,
			OFFLINE_SAMEAS_EXTRA = 47, // seemingly unused by the client
		};

		inline Main::Structures::AccountInfo handleAuthorization(const Common::Network::Packet& request, Main::Network::Session& session, Main::Persistence::PersistentDatabase& database)
		{
			std::uint32_t accountID;
			std::memcpy(&accountID, request.getData(), sizeof(std::uint32_t));
			std::uint32_t accountHash;
			std::memcpy(&accountHash, request.getData() + sizeof(std::uint32_t), sizeof(std::uint32_t));
			Main::Structures::AccountInfo accountInfo = database.getPlayerInfo(accountID);

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());
			response.setExtra(AuthorizationExtra::SUCCESS);

			if (accountInfo.accountID != accountID)
			{
				response.setExtra(AuthorizationExtra::ACCOUNT_NOT_FOUND);
			}
			else if (accountHash != accountInfo.accountKey)
			{
				response.setExtra(AuthorizationExtra::AUTHORIZATION_FAILED);
			}
			//session.asyncWrite(response);

			return accountInfo;
		}
	}
}

#endif