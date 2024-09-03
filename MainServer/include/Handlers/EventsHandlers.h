#ifndef MAIN_EVENTS_HANDLERS_H
#define MAIN_EVENTS_HANDLERS_H

#include "Network/Packet.h"
#include "../../include/Structures/MainEventsList.h"
#include "../Persistence/MainDatabaseManager.h"
#include "../Classes/Player.h"
#include "../../include/Network/MainSession.h"

namespace Main
{
    namespace Handlers
    {
        // TODO: Remove code duplications
        inline void handleModeEvents(const Common::Network::Packet& request, Main::Network::Session& session, Main::Persistence::PersistentDatabase& database)
        {
            auto eventsList = database.getEventsModeList();
            std::uint32_t totalEvents = eventsList.size();

            const std::size_t messageSize = totalEvents * sizeof(Main::Structures::SingleModeEvent) + sizeof(std::uint32_t);

            Common::Network::Packet response;
            response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
            response.setOrder(202);
            std::vector<std::uint8_t> message(messageSize);
            std::memcpy(message.data(), &totalEvents, sizeof(std::uint32_t));
            std::memcpy(message.data() + sizeof(std::uint32_t), eventsList.data(), totalEvents * sizeof(Main::Structures::SingleModeEvent));
            response.setData(message.data(), message.size());

            session.asyncWrite(response);
        }

        // Handles the events (+100% EXP & MP) for the given maps (e.g. house top)
        inline void handleMapEvents(const Common::Network::Packet& request, Main::Network::Session& session, Main::Persistence::PersistentDatabase& database)
        {
            auto eventsList = database.getEventsMapList();
            std::uint32_t totalEvents = eventsList.size();

            const std::size_t messageSize = totalEvents * sizeof(Main::Structures::SingleMapEvent) + sizeof(std::uint32_t);

            Common::Network::Packet response;
            response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
            response.setOrder(203);
            std::vector<std::uint8_t> message(messageSize);
            std::memcpy(message.data(), &totalEvents, sizeof(std::uint32_t));
            std::memcpy(message.data() + sizeof(std::uint32_t), eventsList.data(), totalEvents * sizeof(Main::Structures::SingleMapEvent));
            response.setData(message.data(), message.size());

            session.asyncWrite(response);
        }
    }
}

#endif