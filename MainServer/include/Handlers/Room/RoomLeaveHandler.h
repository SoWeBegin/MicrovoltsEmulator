#ifndef ROOM_LEAVE_HANDLER_H
#define ROOM_LEAVE_HANDLER_H

#include "../../Network/MainSession.h"
#include "../../Classes/RoomsManager.h"
#include "Network/Packet.h"
#include "../../Network/MainSessionManager.h"
#include "Utils/Logger.h"
#include "Utils/IPC_Structs.h"

namespace Main
{
	namespace Handlers
	{
		enum RoomLeaveExtra
		{
			LEAVE_ERROR = 0, // doesn't leave the room
			LEAVE_NORMAL = 1,
			LEAVE_UNKNOWN = 0x2F, // seems normal exit?
			LEAVE_UNKNOWN1 = 0x15, // seems normal exit?
			LEAVE_UNKNOWN2 = 4, // seems normal exit?
			LEAVE_BREAKROOM = 0x1B,
			LEAVE_KICKED_BY_HOST = 0x2A,
			LEAVE_KICKED_BY_MOD = 0x23,
			LEAVE_VOTEKICK_COARSE_LANGUAGE = 0x27,
		};

		enum ClientExtra
		{
			KICK_PLAYER = 28,
		};

	
		// Notifies other players that the player with uniqueId left the room (if you don't send this: the other player clients still believe you're in the room!)
		// NOTE: This decrements all player indexes inside the client! Call it ONLY AFTER sending the packet to change the host!
		inline void notifyRoomPlayerLeaves(Main::Structures::UniqueId uniqueId, Main::Classes::Room& room)
		{
			Common::Network::Packet response;
			response.setTcpHeader(0, Common::Enums::USER_LARGE_ENCRYPTION); // sessionId set inside .broadcastToRoom()
			response.setOrder(422);
			response.setOption(1); // Unsure whether this is the new hostIDX, or the team of the player that left!
			response.setData(reinterpret_cast<std::uint8_t*>(&uniqueId), sizeof(uniqueId));
			room.broadcastToRoom(response);
		}

		// Checked
		inline void handleRoomLeave(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager, Main::Classes::RoomsManager& roomsManager)
		{
			Utils::Logger& logger = Utils::Logger::getInstance();

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());

			auto roomOpt = roomsManager.getRoomByNumber(session.getRoomNumber());
			if (roomOpt == std::nullopt)
			{
				response.setExtra(RoomLeaveExtra::LEAVE_ERROR);
				session.asyncWrite(response);
				return;
			};
			auto& room = roomOpt->get();

			if (request.getExtra() == ClientExtra::KICK_PLAYER)
			{
				response.setExtra(RoomLeaveExtra::LEAVE_KICKED_BY_HOST);
				Main::Structures::UniqueId uniqueId;
				std::memcpy(&uniqueId, request.getData(), sizeof(uniqueId));
				uniqueId.server = 4;
				auto* targetSession = sessionsManager.getSessionBySessionId(uniqueId.session);
				if (targetSession)
				{
					if (targetSession->getAccountInfo().playerGrade >= session.getAccountInfo().playerGrade) return; // Add error messages, i.e. "cannot kick same or higher grade account"
					room.removePlayer(targetSession, RoomLeaveExtra::LEAVE_KICKED_BY_HOST); // RemovePlayer also takes care of sending the packet to the client.
					room.addKickedPlayer(targetSession->getAccountInfo().accountID);
				}
			}
			else
			{
				response.setExtra(RoomLeaveExtra::LEAVE_NORMAL);

				// If there are no players left after this player left, or if there were errors while switching to a new host, just close the room to avoid further issues.
				const bool mustRoomBeClosed = room.removePlayer(&session, RoomLeaveExtra::LEAVE_NORMAL);
				// nb. removePlayer does NOT notify cast server whether room must be closed, so we do it here.

				if (mustRoomBeClosed)
				{
					roomsManager.removeRoom(room.getRoomNumber());
				}
			}
		}
	}
}

#endif