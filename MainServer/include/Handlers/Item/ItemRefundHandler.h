#ifndef ITEM_REFUND_HANDLER_H
#define ITEM_REFUND_HANDLER_H

#include "../../Network/MainSession.h"
#include "../../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "../../../include/Structures/ClientData/Structures.h"
#include "Network/Packet.h"
#include "../../../include/MainEnums.h"

#include <iostream>
namespace Main
{
	namespace Handlers
	{
		inline void handleItemRefund(const Common::Network::Packet& request, Main::Network::Session& session, const Main::ClientData::ItemRefund& itemRefund)
		{
			std::uint32_t toRefund = 0;
			for (const auto& [itemNum, actualItem] : session.getItems())
			{
				if (itemNum == itemRefund.serialInfo.itemNumber)
				{
					Main::ConstantDatabase::CdbUtil cdbUtil(actualItem.id);
					auto refund = cdbUtil.getItemRefundPrice();
					if (refund == std::nullopt) return;
					toRefund = *refund;
				}
			}

			const bool removed = session.deleteItemBasic(itemRefund.serialInfo);
			if (removed)
			{
				session.setAccountMicroPoints(session.getAccountInfo().microPoints + toRefund);
			}

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setCommand(request.getOrder(), 0, removed ? Main::Enums::SellItemExtra::SELL_SUCCESS : Main::Enums::SellItemExtra::SELL_ERROR, 0);
			response.setData(const_cast<std::uint8_t*>(request.getData()), sizeof(Main::Structures::ItemSerialInfo) + sizeof(std::uint32_t));
			session.asyncWrite(response);
		}
	}
}

#endif