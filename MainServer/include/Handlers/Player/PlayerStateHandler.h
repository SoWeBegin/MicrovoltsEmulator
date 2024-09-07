#ifndef PLAYER_STATE_HANDLER_H
#define PLAYER_STATE_HANDLER_H

#include "../../Network/MainSession.h"
#include "../../Network/MainSessionManager.h"
#include "Network/Packet.h"
#include <ConstantDatabase/Structures/SetItemInfo.h>
#include <Utils/Utils.h>

namespace Main
{
	namespace Handlers
	{
		inline void handlePlayerState(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager)
		{
			if (session.getPlayerState() == Common::Enums::PlayerState::STATE_TRADE) return;
			session.setPlayerState(static_cast<Common::Enums::PlayerState>(request.getOption()));

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);

			if (static_cast<Common::Enums::PlayerState>(request.getOption()) == Common::Enums::PlayerState::STATE_CAPSULE)
			{
				session.sendCurrency();
				response.setOrder(83);
				Main::ConstantDatabase::CdbUtil cdbUtil;
				auto capsuleItems = cdbUtil.getCapsuleItems(5); // Total items to show in the capsule
				response.setData(reinterpret_cast<std::uint8_t*>(capsuleItems.data()), capsuleItems.size() * sizeof(Main::Structures::CapsuleList));
				response.setOption(capsuleItems.size());
				session.asyncWrite(response);
			}

			auto foundRoom = roomsManager.getRoomByNumber(session.getRoomNumber());
			if (foundRoom == std::nullopt) return;
			auto& room = foundRoom->get();
			const auto& accountInfo = session.getAccountInfo();

			response.setOrder(312); // Notify other players in the same room about our current state
			response.setOption(session.getPlayerState());
			auto uniqueId = accountInfo.uniqueId;
			response.setData(reinterpret_cast<std::uint8_t*>(&uniqueId), sizeof(uniqueId));
			room.broadcastToRoom(response);
			room.setStateFor(uniqueId, session.getPlayerState());

			// Update other players in the room about our new items / character
			if (static_cast<Common::Enums::PlayerState>(request.getOption()) == Common::Enums::PlayerState::STATE_READY
				|| static_cast<Common::Enums::PlayerState>(request.getOption()) == Common::Enums::PlayerState::STATE_WAITING)
			{
				if (session.getRoomNumber())
				{
					room.updatePlayerInfo(&session);

					using setItems = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::SetItemInfo>;
					struct Resp
					{
						Main::Structures::UniqueId uniqueId;
						Main::Structures::BasicEquippedItem items;
					} resp;
					resp.uniqueId = accountInfo.uniqueId;

					for (const auto& [type, item] : session.getEquippedItems())
					{
						if (type >= Common::Enums::ItemType::SET)
						{
							auto entry = setItems::getInstance().getEntry("si_id", (item.id >> 1));
							if (entry != std::nullopt)
							{
								for (auto currentTypeNotNull : Common::Utils::getPartTypesWhereSetItemInfoTypeNotNull(*entry))
								{
									resp.items.equippedItems[currentTypeNotNull].equippedItemId = item.id;
									resp.items.equippedItems[currentTypeNotNull].type = currentTypeNotNull;
								}
							}
						}
						else if (type >= Common::Enums::ItemType::HAIR && type <= Common::Enums::ItemType::ACC_BACK)
						{
							resp.items.equippedItems[type] = item;
						}
						else if (type >= Common::Enums::ItemType::MELEE && type <= Common::Enums::ItemType::GRENADE)
						{
							resp.items.equippedWeapons[type - 10] = item;
						}
					}
					response.setOrder(414);
					response.setOption(17);
					response.setExtra(accountInfo.latestSelectedCharacter); 
					response.setData(reinterpret_cast<std::uint8_t*>(&resp), sizeof(resp));
					roomsManager.broadcastToRoomExceptSelf(session.getRoomNumber(), accountInfo.uniqueId, response);
				}
			}
		}
	}
}

#endif
