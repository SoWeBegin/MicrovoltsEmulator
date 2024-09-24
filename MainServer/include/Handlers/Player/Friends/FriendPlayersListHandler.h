#ifndef MAIN_FRIENDPLAYER_LIST_HANDLER_H
#define MAIN_FRIENDPLAYER_LIST_HANDLER_H

#include "../../../../include/Network/MainSession.h"
#include "../../../../include/Network/MainSessionManager.h"
#include "../../../Structures/PlayerLists/Friend.h"
#include "Network/Packet.h"

namespace Main
{
    namespace Handlers
    {
		inline void handleFriendList(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager, Main::Persistence::PersistentDatabase& database)
		{
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);

			const auto& pendingRequests = database.loadPendingFriendRequests(session.getAccountInfo().accountID);
			if (!pendingRequests.empty())
			{
				response.setCommand(61, 0, Main::Enums::AddFriendServerExtra::SEND_REQUEST_TO_TARGET, 2);

				for (auto currentPendingFriend : pendingRequests)
				{
					currentPendingFriend.targetUniqueId = Main::Structures::UniqueId{ 0, 4 }; // ignored later, but needed because if we send an empty UniqueId the client won't respond with packet 61
					response.setData(reinterpret_cast<std::uint8_t*>(&currentPendingFriend), sizeof(currentPendingFriend));
					session.asyncWrite(response);
				}
			}
			
            std::vector<Main::Structures::Friend> friendlist = session.getFriendlist();
			response.setCommand(request.getOrder(), 0, 37, friendlist.size());
            response.setData(reinterpret_cast<std::uint8_t*>(friendlist.data()), friendlist.size() * sizeof(Main::Structures::Friend));
            session.asyncWrite(response);
        }
    }
}

#endif