#ifndef ROOM_SCORE_UPDATE_HANDLER
#define ROOM_SCORE_UPDATE_HANDLER

#include "../../Network/MainSession.h"
#include "Network/Packet.h"
#include "../../Classes/RoomsManager.h"
#include <array>
#include <random>

namespace Main
{
    namespace Handlers
    {
        inline void handleBattery(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager,
            const Main::Structures::UniqueId& targetUniqueId)
        {
            if (Main::Classes::Room* room = roomsManager.getRoomByNumber(session.getRoomNumber()))
            {
                std::array<std::uint32_t, 8> weightedBatteries = { 0, 0, 0, 0, 30, 30, 50, 100 };

                static std::random_device rd;
                static std::mt19937 gen(rd());
                static std::uniform_int_distribution<std::size_t> dis(0, weightedBatteries.size() - 1);
                std::uint32_t selectedBattery = weightedBatteries[dis(gen)];

                if (!selectedBattery) return; // No battery needs to be given

                auto* targetSession = room->getPlayer(targetUniqueId);
                if (targetSession)
                {
                    Common::Network::Packet response;
                    response.setTcpHeader(targetSession->getId(), Common::Enums::USER_LARGE_ENCRYPTION);
                    response.setCommand(request.getOrder(), 0, 1, selectedBattery);
                    response.setData(reinterpret_cast<std::uint8_t*>(&selectedBattery), sizeof(selectedBattery));
                    targetSession->asyncWrite(response);
                    targetSession->addBatteryObtainedInMatch(selectedBattery);
                }
            }
        }
    }
}

#endif
