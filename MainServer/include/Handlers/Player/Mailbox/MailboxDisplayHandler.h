#ifndef MAILBOX_DISPLAYER_HANDLER_H
#define MAILBOX_DISPLAYER_HANDLER_H

#include "../../../Network/MainSession.h"
#include "../../../Network/MainSessionManager.h"
#include "Network/Packet.h"
#include "../../../MainEnums.h"
#include "../../../Structures/Mailbox.h"

#include <algorithm>
#include <cstring>
#include "../../../Network/MainSessionManager.h"

namespace Main
{
	namespace Handlers
	{
		inline void handleMailboxDisplay(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager,
            Main::Persistence::PersistentDatabase& database)
		{
            /*
            Common::Network::Packet response;
            response.setTcpHeader(session.getId(), Common::Enums::USER_LARGE_ENCRYPTION);
            response.setOrder(request.getOrder();
            response.setOption(1); // Num of mailboxes
            Main::Structures::Mailbox mailbox;
            mailbox.timestamp = static_cast<__time32_t>(std::time(0));
            mailbox.accountId = 1;
            mailbox.hasBeenRead = false;
            response.setData(reinterpret_cast<std::uint8_t*>(&mailbox), sizeof(mailbox));

			if (request.getMission() == 0)
			{
				// incoming mailboxes
                response.setMission(0);
                session.asyncWrite(response);
                response.setExtra(51);
                session.asyncWrite(response);
                return;
			}
            else if (request.getMission() == 1)
            {
                // sent mailboxes
                response.setMission(1);
                session.asyncWrite(response);
                response.setExtra(51);
                session.asyncWrite(response);
                return;
            }
            */

            auto actualMailbox = request.getMission() == 0 ? session.getMailboxReceived() : session.getMailboxSent();
            auto mailboxData = actualMailbox.data();
            auto accountID = session.getAccountInfo().accountID;
            for (auto& currentMailbox : actualMailbox)
            {
                currentMailbox.hasBeenRead = true;
                database.updateReadMailbox(accountID, currentMailbox.timestamp);
            }

            Common::Network::Packet response;
            response.setTcpHeader(session.getId(), Common::Enums::USER_LARGE_ENCRYPTION);
            response.setOrder(request.getOrder());
            response.setMission(request.getMission());
            response.setOption(actualMailbox.size());

            if (actualMailbox.empty())
            {
                response.setData(nullptr, 0);
                response.setExtra(6);
                session.asyncWrite(response);
                return;
            }
          
            const std::size_t totalBytes = actualMailbox.size() * sizeof(Main::Structures::Mailbox);
            constexpr std::size_t MAX_PACKET_SIZE = 1440;
            constexpr std::size_t headerSize = sizeof(Common::Protocol::TcpHeader) + sizeof(Common::Protocol::CommandHeader);

            if (totalBytes < MAX_PACKET_SIZE)
            {
                response.setExtra(37);
                response.setData(reinterpret_cast<std::uint8_t*>(const_cast<Main::Structures::Mailbox*>(mailboxData)), totalBytes);
                session.asyncWrite(response);
                response.setExtra(51);
                session.asyncWrite(response);
                return;
            }
            else
            {
                std::size_t currentMailboxIdx = 0;
                std::uint16_t packetExtra = 0;
                const std::size_t maxPayloadSize = MAX_PACKET_SIZE - headerSize;
                const std::size_t mailboxesToSend = maxPayloadSize / sizeof(Main::Structures::Mailbox);

                while (currentMailboxIdx < actualMailbox.size())
                {
                    std::vector<Main::Structures::Mailbox> currentMailboxPacket(
                        actualMailbox.begin() + currentMailboxIdx,
                        actualMailbox.begin() + std::min(currentMailboxIdx + mailboxesToSend, actualMailbox.size())
                    );

                    packetExtra = currentMailboxIdx == 0 ? 37 : 0;
                    response.setExtra(packetExtra);
                    response.setData(reinterpret_cast<std::uint8_t*>(currentMailboxPacket.data()), currentMailboxPacket.size() * sizeof(Main::Structures::Mailbox));
                    response.setOption(currentMailboxPacket.size());
                    session.asyncWrite(response);

                    currentMailboxIdx += mailboxesToSend;
                }
                response.setExtra(51);
                session.asyncWrite(response);
            }
		}
	}
}

#endif