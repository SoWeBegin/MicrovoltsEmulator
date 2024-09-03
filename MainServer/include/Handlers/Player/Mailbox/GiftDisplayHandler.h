#ifndef MAILBOX_GIFT_DISPLAY_HANDLER_H
#define MAILBOX_GIFT_DISPLAY_HANDLER_H

#include "../../../Network/MainSession.h"
#include "../../../Network/MainSessionManager.h"
#include "Network/Packet.h"
#include "../../../MainEnums.h"
#include "../../../Structures/Mailbox.h"

#include <algorithm>
#include <cstring>

namespace Main
{
    namespace Handlers
    {
        inline void handleMailboxGiftDisplay(const Common::Network::Packet& request, Main::Network::Session& session)
        {
			// todo, this is just a test 
			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());
			response.setExtra(37);
			response.setMission(0); // sent = 1, received = 0
			response.setOption(1);  // total items in mailbox
			Main::Structures::Giftbox giftTest{};
			giftTest.timestamp =static_cast<__time32_t>(std::time(0));
			giftTest.accountId = session.getAccountInfo().accountID;
			response.setData(reinterpret_cast<std::uint8_t*>(&giftTest), sizeof(giftTest));
			session.asyncWrite(response);
			response.setExtra(51);
			session.asyncWrite(response);

        }
    }
}

#endif