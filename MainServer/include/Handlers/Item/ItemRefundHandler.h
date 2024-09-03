#ifndef ITEM_REFUND_HANDLER_H
#define ITEM_REFUND_HANDLER_H

#include "../../Network/MainSession.h"
#include "../../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "Network/Packet.h"
#include "../../../include/MainEnums.h"

namespace Main
{
	namespace Handlers
	{
		inline void handleItemRefund(const Common::Network::Packet& request, Main::Network::Session& session)
		{
			std::uint32_t mpToAdd{};
			std::memcpy(&mpToAdd, request.getData() + 8, sizeof(std::uint32_t)); 
			Main::Structures::ItemSerialInfo serialInfo{};
			std:memcpy(&serialInfo, request.getData(), sizeof(Main::Structures::ItemSerialInfo));

			const bool removed = session.deleteItemBasic(serialInfo);
			session.setAccountMicroPoints(session.getAccountInfo().microPoints + mpToAdd);

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());
			response.setData(const_cast<std::uint8_t*>(request.getData()), sizeof(Main::Structures::ItemSerialInfo) + sizeof(std::uint32_t));
			response.setExtra(removed ? Main::Enums::SellItemExtra::SELL_SUCCESS : Main::Enums::SellItemExtra::SELL_ERROR);
			session.asyncWrite(response);
		}
	}
}

#endif