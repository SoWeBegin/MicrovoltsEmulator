#ifndef ROOM_INVITE_JOIN_HANDLER_H
#define ROOM_INVITE_JOIN_HANDLER_H

#include "../../Network/MainSession.h"
#include "../../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "../../../include/Structures/Room/RoomSettingsUpdate.h"
#include "Network/Packet.h"
#include <span>
#include "../Room/RoomJoinHandler.h"
#include "../Room/RoomLeaveHandler.h"
#include "../../Utilities.h"
#include "../../Structures/ClientData/Structures.h"


namespace Main
{
	namespace Handlers
	{
		enum RoomInviteJoinExtra
		{
			USER_INVITING_FRIEND = 0,
			USER_READY = 5,
			USER_OFFLINE = 0xD,
			ROOM_FULL = 0xE,
			ROOM_SENDINVITE_TOTARGET = 44,
			USER_JOINING_FRIEND = 28,
		};

		// Room invites/join, not fully implemented, also not working in certain scenarios, recheck + refactor this function
		inline void handleJoinAndInvites(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager,
			Main::Network::SessionsManager& sessionsManager)
		{
			auto response = request;
			if (request.getExtra() == RoomInviteJoinExtra::USER_JOINING_FRIEND)
			{
				std::uint32_t targetAccountId;
				std::memcpy(&targetAccountId, reinterpret_cast<std::uint8_t*>(const_cast<std::uint8_t*>(request.getData())), sizeof(targetAccountId));

				if (auto* targetSession = sessionsManager.getSessionByAccountId(targetAccountId))
				{
					const std::uint32_t targetRoomNum = targetSession->getRoomNumber();
					const std::uint32_t selfRoomNum = session.getRoomNumber();
					if (targetRoomNum)
					{
						if (!session.isInLobby() && !selfRoomNum)
						{ // User is neither in lobby and in room, join should not work in this case
							Main::Details::sendMessage("You can only join a friend's room while being in a different room or in the lobby!", session);
							return;
						}
						else if (targetRoomNum == selfRoomNum)
						{ // User attempting to join the same room, disallow this
							Main::Details::sendMessage("You cannot join this friend as you're already in their room.", session);
							return;
						}
						else if (selfRoomNum)
						{ // if the player is in a room, leave it first
							response.setCommand(137, 0, 0, 0);
							Main::Handlers::handleRoomLeave(response, session, sessionsManager, roomsManager, 4, Details::parseData<Main::Structures::UniqueId>(response));
						}

						Main::ClientData::RoomInfo joinInfo{ targetRoomNum - 1 };
						response.setCommand(136, 0, 0, 0);
						response.setData(reinterpret_cast<std::uint8_t*>(&joinInfo), sizeof(joinInfo));

						if (Main::Classes::Room* room = roomsManager.getRoomByNumber(targetRoomNum))
						{
							Main::Handlers::handleRoomJoin(response, session, roomsManager, joinInfo, !room->getPassword().empty());
						}
						else
						{
							Main::Details::sendMessage("Room not found.", session);
						}
					}
					else
					{
						Main::Details::sendMessage("This friend is currently not inside a room.", session);
						return;
					}
				}
				else
				{
					Main::Details::sendMessage("The friend you're trying to join went offline.", session);
					return;
				}
			}
			else if (request.getExtra() == RoomInviteJoinExtra::USER_INVITING_FRIEND && request.getOption() == 2) // Currently doesn't work, something's wrong here
			{
				char targetNickname[16];
				std::ranges::copy(std::span(reinterpret_cast<const char*>(request.getData()), sizeof(targetNickname)), targetNickname);

				Main::Network::Session* targetSession = sessionsManager.findSessionByName(targetNickname);
				if (targetSession)
				{
					if (!targetSession->isInLobby())
					{
						Details::sendMessage("This player is currently not in the lobby.", session);
					}
					else if (targetSession->getRoomNumber() == session.getRoomNumber())
					{
						Details::sendMessage("The player you are trying to invite is already in your room.", session);
					}
					if (Main::Classes::Room* room = roomsManager.getRoomByNumber(session.getRoomNumber()))
					{
						response.setCommand(319, 0, 0, 0);
						response.setSession(targetSession->getId());

#pragma pack(push, 1)
						struct Response
						{
							std::uint32_t serverId = 4; // unsure whether this really is the server's id
							char sourceNickname[16]{}; // OK
							std::uint16_t roomNumber{};
							std::uint16_t unknown{ 2 };
							char roomTitle[32]{};
						};
#pragma pack(pop)

						Response responseStruct;
						responseStruct.roomNumber = session.getRoomNumber() - 1; // correct
						std::ranges::copy(std::span(room->getRoomTitle().c_str(), sizeof(responseStruct.roomTitle) - 1), responseStruct.roomTitle);
						responseStruct.roomTitle[sizeof(responseStruct.roomTitle) - 1] = '\0';
						std::ranges::copy(std::span(session.getAccountInfo().nickname, sizeof(responseStruct.sourceNickname) - 1), responseStruct.sourceNickname);
						responseStruct.sourceNickname[sizeof(responseStruct.sourceNickname) - 1] = '\0';
						response.setData(reinterpret_cast<std::uint8_t*>(&responseStruct), sizeof(responseStruct));
						targetSession->asyncWrite(response);
					}
					else
					{
						Details::sendMessage("You must be in a room to invite a player.", session);
					}
				}
				else
				{
					Details::sendMessage("The player you invited is offline.", session);
				}
			}
			else if (session.getRoomNumber())
			{
				roomsManager.broadcastToRoom(session.getRoomNumber(), const_cast<Common::Network::Packet&>(request));
			}
		}
	}
}

#endif