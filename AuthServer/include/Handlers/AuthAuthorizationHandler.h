#ifndef AUTH_AUTHORIZATION_HANDLER_H
#define AUTH_AUTHORIZATION_HANDLER_H

#include <string>
#include <functional>
#include "Network/Session.h"
#include "Network/Packet.h"
#include "../Database/DbPlayerInfo.h"
#include "../Player/AuthPlayerManager.h"
#include <limits>

namespace Auth
{
	namespace Handlers
	{
		inline void handleAuthUserInformation(const Common::Network::Packet& request, Common::Network::Session& session, Auth::Persistence::PersistentDatabase& database)
		{
			const std::string username = std::string(reinterpret_cast<const char*>(request.getData() + 48));
			const std::string password = std::string(reinterpret_cast<const char*>(request.getData() + 4));
			auto pair = database.getPlayerInfo(username, password);

			Common::Network::Packet response = pair.first;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(22);
			response.setMission(0);
			// note: getPlayerInfo sets command.extra, command.option and message data
			
			if (response.getExtra() != Auth::Enums::Login::SUCCESS)
			{
				response.setData(nullptr, 0); 
				session.asyncWrite(response);
				return;
			}

			session.asyncWrite(response);
			Auth::Player::AuthPlayerManager::getInstance().addPlayer(session.getId(), pair.second);
		}
	}
}

#endif