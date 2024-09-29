#ifndef ROOM_SCORE_UPDATE_HANDLER
#define ROOM_SCORE_UPDATE_HANDLER

#include "../../Network/MainSession.h"
#include "Network/Packet.h"
#include "../../Classes/RoomsManager.h"
#include <array>

namespace Main
{
	namespace Handlers
	{
        inline void handleBattery(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager)
        {
            auto room = roomsManager.getRoomByNumber(session.getRoomNumber());
            if (room == std::nullopt) return;
            auto& actualRoom = room->get();

            auto response = request;
            std::srand(static_cast<unsigned>(std::time(nullptr)));

            std::vector<std::uint32_t> weightedBatteries = { 0, 0, 0, 0, 30, 30, 50, 100 };

            std::uint32_t randomIndex = std::rand() % weightedBatteries.size();
            std::uint32_t selectedBattery = weightedBatteries[randomIndex];

            if (selectedBattery == 0) return; // No battery needs to be given

            Main::Structures::UniqueId targetUniqueId;
            auto data = request.getData();
            std::memcpy(&targetUniqueId, data, sizeof(targetUniqueId));

            auto* targetSession = actualRoom.getPlayer(targetUniqueId);
            if (targetSession)
            {
                response.setTcpHeader(targetSession->getId(), Common::Enums::USER_LARGE_ENCRYPTION);
                response.setOrder(request.getOrder());
                response.setExtra(1);
                response.setOption(selectedBattery);
                response.setMission(0);
                response.setData(nullptr, 0);

                std::uint32_t receivedBattery = selectedBattery;
                response.setData(reinterpret_cast<std::uint8_t*>(&receivedBattery), sizeof(receivedBattery)); 
                targetSession->asyncWrite(response);
                targetSession->addBatteryObtainedInMatch(selectedBattery);
            }
        }
	}
}

#endif