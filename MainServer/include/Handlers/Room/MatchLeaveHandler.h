#ifndef MATCH_LEAVE_HANDLER_H
#define MATCH_LEAVE_HANDLER_H

#include "../../Network/MainSession.h"
#include "../../Classes/RoomsManager.h"
#include "Network/Packet.h"
#include "../../Network/MainSessionManager.h"
#include "Utils/Logger.h"

namespace Main
{
	namespace Handlers
	{	
		// Checked
		inline void handleMatchLeave(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager, Main::Classes::RoomsManager& roomsManager)
		{
			Utils::Logger& logger = Utils::Logger::getInstance();

			auto foundRoom = roomsManager.getRoomByNumber(session.getRoomNumber());
			if (foundRoom == std::nullopt) 
			{
				logger.log("Room not found by number!, Utils::LogType::Error, Main::handleMatchLeave");
				return;
			}
			auto& room = foundRoom->get();

			if (room.isHost(session.getAccountInfo().uniqueId))
			{
				logger.log("The host " + session.getPlayerInfoAsString() + " is attempting to leave the match. " + room.getRoomInfoAsString(),
					Utils::LogType::Normal, "Main::handleMatchLeave");

				bool roomMustBeClosed = room.removeHostFromMatch();
				if (roomMustBeClosed)
				{
					logger.log("Attempting to close the room " + room.getRoomInfoAsString(),
						Utils::LogType::Normal, "Main::handleMatchLeave");

					roomsManager.removeRoom(room.getRoomNumber());
				}
			}
			else
			{
				logger.log("The player " + session.getPlayerInfoAsString() + " is attempting to leave the match. " + room.getRoomInfoAsString(),
					Utils::LogType::Normal, "Main::handleMatchLeave");

				// Send leave match packet to client 
				Common::Network::Packet response;
				response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
				response.setOrder(request.getOrder());
				response.setExtra(0);
				auto uniqueId = session.getAccountInfo().uniqueId;
				response.setData(reinterpret_cast<std::uint8_t*>(&uniqueId), sizeof(uniqueId));
				roomsManager.broadcastToRoom(room.getRoomNumber(), response);

				// Update server status for this player
				session.setIsInMatch(false);
				room.setStateFor(session.getAccountInfo().uniqueId, Common::Enums::STATE_WAITING);

				// This is NOT necessary: when the user clicks the sign of end match, they leave the match. Check which packet that is.
				//room.removeHostIfAloneAndModeDoesntAllowIt();

				logger.log("The player " + session.getPlayerInfoAsString() + " has left the match. " + room.getRoomInfoAsString(),
					Utils::LogType::Normal, "Main::handleMatchLeave");
			}
		}
	}
}

#endif
