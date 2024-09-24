#ifndef ACCOUNT_INFO_IN_LOBBY_HANDLER_H
#define ACCOUNT_INFO_IN_LOBBY_HANDLER_H

#include "Network/Session.h"
#include "Network/Packet.h"
#include "../../Structures/AccountInfo/MainLobbyAccountInfo.h"
#include "../../MainEnums.h"
#include "../../../include/Network/MainSession.h"
#include "../../../include/Network/MainSessionManager.h"
#include <ConstantDatabase/Structures/SetItemInfo.h>

namespace Main
{
	namespace Handlers
	{
		inline void handleLobbyAccountInfo(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager, 
			std::uint32_t serverId)
		{
			Main::Structures::UniqueId uniqueId;
			std::memcpy(&uniqueId, request.getData(), sizeof(Main::Structures::UniqueId));
			auto* targetSession = sessionsManager.getSessionBySessionId(uniqueId.session);
			if (!targetSession) return;

			Main::Structures::LobbyAccountInfo lobbyAccountInfo(targetSession->getAccountInfo());
			using setItems = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::SetItemInfo>;
			for (const auto& [type, item] : targetSession->getEquippedItems())
			{
				if (type >= Common::Enums::SET)
				{
					auto entry = setItems::getInstance().getEntry("si_id", (item.id >> 1));
					if (entry != std::nullopt)
					{
						for (auto currentTypeNotNull : Common::Utils::getPartTypesWhereSetItemInfoTypeNotNull(*entry))
						{
							lobbyAccountInfo.items[currentTypeNotNull] = (item.id >> 1);
						}
					}
				}
				else lobbyAccountInfo.items[type] = (item.id >> 1);
			}
		
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setCommand(request.getOrder(), 0, targetSession->getAccountInfo().clanId >= 8 ? 1 : 0, serverId);
			response.setData(reinterpret_cast<std::uint8_t*>(&lobbyAccountInfo), sizeof(Main::Structures::LobbyAccountInfo));
			session.asyncWrite(response);
		}
	}
}

#endif
