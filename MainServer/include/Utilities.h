#ifndef DETAILS_HEADER_H
#define DETAILS_HEADER_H

#include <ConstantDatabase/Structures/SetItemInfo.h>
#include <Utils/Utils.h>
#include "Network/MainSession.h"
#include "Classes/RoomsManager.h"
#include "Network/Packet.h"

namespace Main
{
	namespace Details
	{
		namespace Constants
		{
			constexpr inline std::uint32_t maxEquippedItems = 17;
		}

		enum Orders
		{
			PLAYER_STATE_NOTIFICATION = 312,
			PLAYER_ITEMS_BROADCAST = 414,
		};

		inline void sendMessage(const std::string& message, Main::Network::Session& session)
		{
			Common::Network::Packet response;
			response.setTcpHeader(session.getId(), Common::Enums::USER_LARGE_ENCRYPTION);
			std::string m_confirmationMessage{ std::string(16, '0') };
			m_confirmationMessage += message;
			response.setOrder(316);
			response.setExtra(1);
			response.setData(reinterpret_cast<std::uint8_t*>(m_confirmationMessage.data()), m_confirmationMessage.size());
			response.setOption(m_confirmationMessage.size());
			session.asyncWrite(response);
		}

		inline void broadcastPlayerItems(Main::Classes::RoomsManager& roomsManager, Main::Network::Session& session, const Common::Network::Packet& request)
		{
			auto foundRoom = roomsManager.getRoomByNumber(session.getRoomNumber());
			if (foundRoom)
			{
				auto& room = foundRoom->get();
				room.updatePlayerInfo(&session);

				using setItems = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::SetItemInfo>;
				struct Resp
				{
					Main::Structures::UniqueId uniqueId;
					Main::Structures::BasicEquippedItem items{};
				} resp{};
				const auto accountInfo = session.getAccountInfo();
				resp.uniqueId = accountInfo.uniqueId;

				// reset state -- needed for when the target user unequips an item!
				for (std::uint32_t i = 0; i < Details::Constants::maxEquippedItems; ++i)
				{
					resp.items[i].equippedItemId = 0;
					resp.items[i].type = i;
				}
				for (const auto& [type, item] : session.getEquippedItems())
				{
					if (type >= Common::Enums::ItemType::SET)
					{
						auto entry = setItems::getInstance().getEntry("si_id", (item.id >> 1));
						if (entry != std::nullopt)
						{
							for (auto currentTypeNotNull : Common::Utils::getPartTypesWhereSetItemInfoTypeNotNull(*entry))
							{
								std::cout << "(in set) Item found for type: " << type << '\n';
								resp.items[currentTypeNotNull].equippedItemId = item.id;
								resp.items[currentTypeNotNull].type = currentTypeNotNull;
							}
						}
					}
					else
					{
						std::cout << "Item found for type: " << type << '\n';
						resp.items[type] = item;
					}
				}

				Common::Network::Packet response;
				response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
				response.setOrder(Details::Orders::PLAYER_ITEMS_BROADCAST);
				response.setOption(Details::Constants::maxEquippedItems);
				response.setExtra(accountInfo.latestSelectedCharacter);
				response.setData(reinterpret_cast<std::uint8_t*>(&resp), sizeof(resp));
				roomsManager.broadcastToRoomExceptSelf(session.getRoomNumber(), accountInfo.uniqueId, response);
			}
		}

		inline std::uint32_t getRandomBoxItemId(std::uint32_t boxId, std::uint32_t boxRareItemId)
		{
			using Map = std::unordered_map<std::uint32_t, Main::Enums::BoxItemsCommon>;
			using BoxItem = Main::Enums::BoxItemsCommon;

			const static Map commonItemProb
			{
				{2000, BoxItem::MP_100},      // 20% 
				{1500, BoxItem::MP_500},      // 15% 
				{1200, BoxItem::MP_1000},     // 12% 
				{1000, BoxItem::MP_2500},     //... etc.
				{800,  BoxItem::MP_3000},     
				{700,  BoxItem::MP_4000},     
				{600,  BoxItem::MP_5000}, 
				{500,  BoxItem::MP_7000},     
				{400,  BoxItem::MP_10000},    
				{300,  BoxItem::MP_20000},   
				{5,    BoxItem::MP_50000},    
				{1,    BoxItem::MP_100000},   
				{1,    BoxItem::MP_500000},   
			};

			Map boxProbabilities = commonItemProb;
			constexpr std::uint32_t rareItemProbability = 300; 
			boxProbabilities[rareItemProbability] = static_cast<BoxItem>(boxRareItemId);

			std::vector<std::pair<std::uint32_t, Main::Enums::BoxItemsCommon>> cumulativeItems;
			std::uint32_t sum = 0;

			for (const auto& [probability, itemID] : boxProbabilities)
			{
				sum += probability;
				cumulativeItems.emplace_back(sum, itemID);
			}

			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> dis(0, sum - 1);

			auto it = std::lower_bound(cumulativeItems.begin(), cumulativeItems.end(),
				std::make_pair(dis(gen), Main::Enums::BoxItemsCommon(0)),
				[](const auto& lhs, const auto& rhs) 
				{
					return lhs.first < rhs.first;
				});

			return static_cast<std::uint32_t>(it->second); 
		}
	}
}

#endif