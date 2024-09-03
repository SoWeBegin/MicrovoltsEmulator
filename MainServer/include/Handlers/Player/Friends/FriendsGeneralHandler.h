#ifndef FRIENDS_GENERAL_HANDLER_HEADER
#define FRIENDS_GENERAL_HANDLER_HEADER

#include "../../../Network/MainSession.h"
#include "../../../Network/MainSessionManager.h"
#include "../../../MainEnums.h"
#include "../../../Structures/PlayerLists/Friend.h"
#include "Network/Packet.h"

#include <algorithm>
#include <cstring>

namespace Main
{
	namespace Handlers
	{
		inline void handleGeneralFriendRequests(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager,
			Main::Persistence::PersistentDatabase& database)
		{
			const auto& accountInfo = session.getAccountInfo();
			const auto* const data = request.getData();

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(request.getOrder());

			constexpr const std::uint32_t maxFriends = 30;

			switch (request.getExtra())
			{
				case Main::Enums::ClientFriendExtra::FRIEND_REQUEST_SENT:
				{
					if (request.getOption() != 2) break; // option seems to always be 2 when a friend request is sent

					if (session.getFriendlist().size() > maxFriends)
					{
						response.setExtra(Main::Enums::AddFriendServerExtra::TARGET_OR_SENDER_FRIEND_LIST_FULL);
						response.setMission(Main::Enums::AddFriendServerMission::SENDER_FRIENDLIST_FULL);
						session.asyncWrite(response);
						return;
					}

					auto* targetSession = sessionsManager.findSessionByName(reinterpret_cast<const char*>(data));
					if (!targetSession) // target offline
					{
						const auto res = database.addPendingFriendRequest(session.getAccountInfo().accountID, reinterpret_cast<const char*>(data));

						if (res == Enums::AddFriendServerExtra::DB_ERROR)
						{
							return;
						}
						if (res != Enums::AddFriendServerExtra::REQUEST_SENT)
						{
							response.setExtra(res);
							if (res == Enums::AddFriendServerExtra::TARGET_OR_SENDER_FRIEND_LIST_FULL)
							{
								response.setMission(Main::Enums::AddFriendServerMission::RECEIVER_FRIENDLIST_FULL);
							}
							session.asyncWrite(response);
							return;
						}
					}
					else
					{
						// The target is online, use their cached information
						if (targetSession->getFriendlist().size() > maxFriends)
						{
							response.setExtra(Main::Enums::AddFriendServerExtra::TARGET_OR_SENDER_FRIEND_LIST_FULL);
							response.setMission(Main::Enums::AddFriendServerMission::RECEIVER_FRIENDLIST_FULL);
							session.asyncWrite(response);
							return;
						}
						else if (targetSession->hasBlocked(accountInfo.accountID))
						{
							response.setExtra(Main::Enums::AddFriendServerExtra::RECEIVER_BLOCKED_SENDER);
							session.asyncWrite(response);
							return;
						}
						else
						{
							response.setExtra(Main::Enums::AddFriendServerExtra::SEND_REQUEST_TO_TARGET);
							Main::Structures::Friend friendStruct{ accountInfo.uniqueId, accountInfo.accountID };
							std::memcpy(friendStruct.targetNickname, session.getPlayerName(), 16);
							response.setData(reinterpret_cast<std::uint8_t*>(&friendStruct), sizeof(friendStruct));
							targetSession->asyncWrite(response);
							return;
						}
					}
				}
				break;

				case Main::Enums::ClientFriendExtra::INCOMING_FRIEND_REQUEST_ACCEPTED:
				{
					std::uint32_t accountId;
					std::memcpy(&accountId, data + sizeof(accountInfo.uniqueId), sizeof(accountInfo.accountID));
					auto* senderSession = sessionsManager.getSessionByAccountId(accountId);

					response.setData(const_cast<std::uint8_t*>(data), sizeof(accountInfo.uniqueId) + sizeof(accountInfo.accountID) + sizeof(accountInfo.nickname));
					response.setExtra(Main::Enums::AddFriendServerExtra::REQUEST_ACCEPTED);

					if (!senderSession) // only the receiver is online
					{
						// In this case just update the Friendlist for the target in the database, using its accountID
						Main::Structures::Friend target{ {}, accountId };
						std::memcpy(target.targetNickname, data + sizeof(accountInfo.uniqueId) + sizeof(accountInfo.accountID), 16);
						session.addFriend(target);
						session.asyncWrite(response);
					}
					else // sender and receiver are both online
					{
						senderSession->addOnlineFriend(&session);
						session.addOnlineFriend(senderSession);
						session.asyncWrite(response);

						Main::Structures::Friend ffriend{ accountInfo.uniqueId, accountInfo.accountID };
						std::memcpy(ffriend.targetNickname, accountInfo.nickname, 16);
						response.setData(reinterpret_cast<std::uint8_t*>(&ffriend), sizeof(ffriend));
						senderSession->asyncWrite(response);
					}
				}
			}
		}
	}
}

#endif