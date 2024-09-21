#ifndef MAIN_ACCOUNT_INVENTORY_HANDLER_H
#define MAIN_ACCOUNT_INVENTORY_HANDLER_H

#include "Network/Session.h"
#include "../../MainEnums.h"

#include <unordered_map>

namespace Main
{
    namespace Handlers
    {
        inline void handleInventoryInformation(const Common::Network::Packet& request, Main::Network::Session& session,
            Main::Network::SessionsManager& sessionsManager, Main::Persistence::PersistentDatabase& database)
        {
            std::uint32_t accountID;
            std::memcpy(&accountID, request.getData(), sizeof(std::uint32_t));
          
            auto playerItems = database.getPlayerItems(accountID);
            auto nonEquippedItems = playerItems.first;
            auto equippedItems = playerItems.second;  

            // Handle non-equipped items
            Common::Network::Packet response;
            response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
            response.setOrder(77);
            response.setOption(nonEquippedItems.size());

            constexpr std::size_t headerSize = sizeof(Common::Protocol::TcpHeader) + sizeof(Common::Protocol::CommandHeader);
            std::size_t totalSize = headerSize + nonEquippedItems.size() * sizeof(Main::Structures::Item);
            constexpr std::size_t MAX_PACKET_SIZE = 1440;

            if (totalSize == headerSize)
            {
                response.setData(nullptr, 0);
                response.setExtra(6);
                session.asyncWrite(response);
            }
            else if (totalSize < MAX_PACKET_SIZE)
            {
                response.setExtra(37);
                response.setData(reinterpret_cast<std::uint8_t*>(nonEquippedItems.data()), nonEquippedItems.size() * sizeof(Main::Structures::Item));
                session.asyncWrite(response);
            }
            else
            {
                std::size_t currentItemIndex = 0;
                std::uint16_t packetExtra = 0;
                const std::size_t maxPayloadSize = MAX_PACKET_SIZE - headerSize;
                const std::size_t itemsToSend = maxPayloadSize / sizeof(Main::Structures::Item);

                while (currentItemIndex < nonEquippedItems.size())
                {
                    std::vector<Main::Structures::Item> packetItems(
                        nonEquippedItems.begin() + currentItemIndex,
                        nonEquippedItems.begin() + std::min(currentItemIndex + itemsToSend, nonEquippedItems.size())
                    );

                    packetExtra = currentItemIndex == 0 ? 37 : 0;
                    response.setExtra(packetExtra);
                    response.setData(reinterpret_cast<std::uint8_t*>(packetItems.data()), packetItems.size() * sizeof(Main::Structures::Item));
                    response.setOption(packetItems.size());
                    session.asyncWrite(response);

                    currentItemIndex += itemsToSend;
                }
            }

            session.setUnequippedItems(nonEquippedItems);

            // Handle equipped items
            response.setOrder(75);
            response.setMission(0);

            for (auto& [characterID, items] : equippedItems)
            {
                response.setExtra(characterID);
                response.setOption(items.size());
                response.setData(reinterpret_cast<std::uint8_t*>(items.data()), items.size() * sizeof(Main::Structures::EquippedItem));
                session.asyncWrite(response);
            }

            // Client expects this as confirmation
            response.setOption(0);
            response.setExtra(5);
            response.setData(nullptr, 0);
            session.asyncWrite(response);

            session.setEquippedItems(equippedItems);
        }
    }
}

#endif