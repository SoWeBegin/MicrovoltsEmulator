#ifndef SEAL_ITEM_HANDLER_H
#define SEAL_ITEM_HANDLER_H

#include "Network/Packet.h"
#include "../../../include/Network/MainSession.h"
#include "DeleteItemHandler.h"
#include <random>

namespace Main
{
    namespace Handlers
    {
        inline void handleGambleItem(const Common::Network::Packet& request, Main::Network::Session& session)
        {
           Common::Network::Packet response;
           response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
		   const auto& accountInfo = session.getAccountInfo();

		   constexpr const std::uint32_t gambleCost = 10'000;
		   if (accountInfo.microPoints < gambleCost)
		   {   // send "not enough MP" message
			   response.setOrder(193);
			   response.setExtra(44);
			   session.asyncWrite(response);
			   return;
		   }

		   if (session.getPlayerState() == Common::Enums::STATE_INVENTORY)
		   {
			   Main::Structures::ItemSerialInfo itemSerialInfo;
			   std::memcpy(&itemSerialInfo, request.getData(), sizeof(itemSerialInfo));

			   using cdbItems = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbItemInfo>;
			   using cdbWeapons = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbWeaponInfo>;
			   const auto& itemGambleItems = cdbItems::getGambleItems();
			   const auto& weaponGambleItems = cdbWeapons::getGambleItems();

			   const auto found = session.findItemBySerialInfo(itemSerialInfo);
			   Main::ConstantDatabase::CdbUtil cdbUtil(found->id);
			   if (found == std::nullopt) return;
			   auto itemType = *(cdbUtil.getItemType());

			   std::uint32_t newRandomId{};
			   std::mt19937 gen(std::random_device{}()); 

			   if (itemGambleItems.contains(itemType)) 
			   {
				   std::size_t size = itemGambleItems.at(itemType).size();
				   std::uniform_int_distribution<int> dist(0, size - 1);
				   int randomIndex = dist(gen); 
				   newRandomId = itemGambleItems.at(itemType)[randomIndex];
			   }
			   else if (weaponGambleItems.contains(itemType)) 
			   {
				   std::size_t size = weaponGambleItems.at(itemType).size();
				   std::uniform_int_distribution<int> dist(0, size - 1); 
				   int randomIndex = dist(gen); 
				   newRandomId = weaponGambleItems.at(itemType)[randomIndex];
			   }
			   else
			   {
				   return; 
			   }

			   if (!session.deleteItem(itemSerialInfo)) return;
			   session.spawnItem(newRandomId, itemSerialInfo);
			   session.setAccountMicroPoints(accountInfo.microPoints - gambleCost);
			   session.sendCurrency();
		   }
        }
    }
}

#endif