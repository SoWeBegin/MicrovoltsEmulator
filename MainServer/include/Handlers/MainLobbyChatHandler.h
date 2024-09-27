#ifndef MAIN_CHAT_HANDLER_H
#define MAIN_CHAT_HANDLER_H

#include "Network/Session.h"
#include "Network/Packet.h"
#include "../MainEnums.h"
#include "../ChatCommands/SimpleCommands/SetCurrency.h"
#include "../Classes/RoomsManager.h"
#include "../ChatCommands/ChatCommands.h"

#include <vector>

namespace Main
{
	namespace Handlers
	{
		inline Main::Enums::ChatGrade getChatGrade(Common::Enums::PlayerGrade playerGrade)
		{
			using PlayerGrade = Main::Enums::PlayerGrade;
			using ChatGrade = Main::Enums::ChatGrade;
			if (playerGrade == PlayerGrade::GRADE_NORMAL) return static_cast<ChatGrade>(0);
			else if (playerGrade == PlayerGrade::GRADE_MOD) return static_cast<Main::Enums::ChatGrade>(3);
			return static_cast<Main::Enums::ChatGrade>(2);
		}

		inline bool checkIfMuted(Main::Network::Session& session, std::uint32_t sessionId)
		{
			Common::Network::Packet response;
			response.setTcpHeader(sessionId, Common::Enums::USER_LARGE_ENCRYPTION);
			if (session.isMuted())
			{
				Details::sendMessage("you have been muted by a moderator.", session);
				return true;
			}
			return false;
		}

		inline bool executeCommand(Main::Network::Session& session, const Common::Network::Packet& request, Common::Network::Packet& response,
			Main::Classes::RoomsManager& roomsManager, Main::Command::ChatCommands& chatCommands, Main::Network::SessionsManager& sessionsManager,
			Main::Persistence::PersistentDatabase& database, const Main::Network::Session::AccountInfo& accountInfo)
		{
			if (request.getExtra() == Enums::ChatExtra::COMMAND)
			{
				const std::string command{ reinterpret_cast<const char*>(request.getData() + 1), static_cast<std::size_t>(request.getOption() - 1) };
				if (command == "?")
				{
					chatCommands.showUsages(session, response, static_cast<Common::Enums::PlayerGrade>(accountInfo.playerGrade));
					return true;
				}
				const std::string commandName = command.substr(0, command.find(' '));
				if (!chatCommands.executeSimpleCommand(commandName, session, sessionsManager, response))
				{
					if (!chatCommands.executeComplexCommand(commandName, command, session, sessionsManager, response))
					{
						if (!chatCommands.executeDatabaseCommand(commandName, command, session, database, response))
						{
							const std::uint16_t roomNumber = session.getRoomNumber();
							if (!roomNumber) return true;
							if (!chatCommands.executeSimpleRoomCommand(commandName, session, roomsManager, roomNumber, response))
							{
								if (!chatCommands.executeComplexRoomCommand(commandName, command, session, roomsManager, response));
							}
						}
					}
				}
				return true;
			}
			return false;
		}

		inline void handleWhisperMessage(const Common::Network::Packet& request, Main::Network::Session& session,
			Main::Network::SessionsManager& sessionsManager, Common::Network::Packet& response)
		{
			constexpr std::uint16_t playerNameLength = 16;
			char receiverNickname[16];
			std::memcpy(receiverNickname, request.getData(), playerNameLength);

			auto* targetSession = sessionsManager.findSessionByName(receiverNickname);
			bool inRoom = false;
			if (!targetSession)
			{
				Main::Structures::UniqueId uniqueId;
				auto data = request.getData();
				std::memcpy(&uniqueId, data, sizeof(uniqueId));
				targetSession = sessionsManager.getSessionBySessionId(uniqueId.session);
				inRoom = true;
			}

			const auto& accountInfo = session.getAccountInfo();
			constexpr std::size_t uidSize = sizeof(accountInfo.uniqueId);
			const char* senderNickname = accountInfo.nickname;
			const uint8_t* uid = reinterpret_cast<const uint8_t*>(&(accountInfo.uniqueId));
			const char* message = reinterpret_cast<const char*>(request.getData() + (inRoom ? sizeof(Main::Structures::UniqueId) : playerNameLength));

			std::vector<std::uint8_t> responseData(playerNameLength + request.getOption() + uidSize);
			std::copy(uid, uid + uidSize, responseData.begin());
			std::copy(senderNickname, senderNickname + playerNameLength, responseData.begin() + uidSize);
			std::copy(message, message + request.getOption(), responseData.begin() + playerNameLength + uidSize);

			response.setData(responseData.data(), responseData.size());
			if (targetSession)
			{
				const auto& targetAccountInfo = targetSession->getAccountInfo();
				if (session.hasBlocked(targetAccountInfo.accountID))
				{
					response.setExtra(Enums::WhisperExtra::WHISPER_SENDER_BLOCKED_RECEIVER);
				}
				else if (targetSession->hasBlocked(accountInfo.accountID))
				{
					response.setExtra(Enums::WhisperExtra::WHISPER_RECEIVER_BLOCKED_SENDER);
				}
				else
				{
					sessionsManager.sendTo(targetSession->getId(), response);
					response.setExtra(Enums::WhisperExtra::WHISPER_SENT);
				}
			}
			else
			{
				response.setExtra(Enums::WhisperExtra::RECEIVER_OFFLINE);
			}

			response.setOrder(315); // 315 extra used to notify the sender whether their whisper was sent
			response.setMission(1);  // 1 = WhisperConfirmation
			response.setData(nullptr, 0);
			session.asyncWrite(response);
		}

		inline bool executeCommon(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager,
			Main::Command::ChatCommands& chatCommands, Main::Classes::RoomsManager& roomsManager, Main::Persistence::PersistentDatabase& database,
			const Main::Network::Session::AccountInfo& accountInfo, Common::Network::Packet& response)
		{
			if (checkIfMuted(session, request.getSession()))
			{
				return true;
			}

			if (executeCommand(session, request, response, roomsManager, chatCommands, sessionsManager, database, accountInfo))
			{
				return true;
			}

			constexpr std::uint16_t playerNameLength = 16;
			if (request.getExtra() == Enums::ChatExtra::WHISPER)
			{
				handleWhisperMessage(request, session, sessionsManager, response);
				return true;
			}

			return false;
		}


		inline void handleLobbyChatMessage(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager,
			Main::Command::ChatCommands& chatCommands, Main::Classes::RoomsManager& roomsManager, Main::Persistence::PersistentDatabase& database)
		{
			constexpr std::uint16_t playerNameLength = 16;
			const auto& accountInfo = session.getAccountInfo();

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setCommand(316, getChatGrade(static_cast<Common::Enums::PlayerGrade>(accountInfo.playerGrade)), request.getExtra(), request.getOption());

			if (executeCommon(request, session, sessionsManager, chatCommands, roomsManager, database, accountInfo, response))
			{
				return;
			}

			const char* senderNickname = accountInfo.nickname;
			const char* message = reinterpret_cast<const char*>(request.getData());
			std::vector<std::uint8_t> responseData(playerNameLength + request.getOption());
			std::copy(senderNickname, senderNickname + playerNameLength, responseData.begin());
			std::copy(message, message + request.getOption(), responseData.begin() + playerNameLength);
			response.setData(responseData.data(), responseData.size());

			if (request.getExtra() == Enums::ChatExtra::NORMAL)
			{
				sessionsManager.broadcastToLobbyExceptSelf(session.getId(), response);
			}
			else 
			{
				sessionsManager.broadcastToClan(session.getId(), response);
			}
		}

		inline void handleRoomChatMessage(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager,
			Main::Command::ChatCommands& chatCommands, Main::Classes::RoomsManager& roomsManager, Main::Persistence::PersistentDatabase& database)
		{
			const auto& accountInfo = session.getAccountInfo();
			constexpr std::uint16_t playerNameLength = 16;

			Common::Network::Packet response;
			response.setCommand(316, getChatGrade(static_cast<Common::Enums::PlayerGrade>(accountInfo.playerGrade)), request.getExtra(), request.getOption());
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);

			if (executeCommon(request, session, sessionsManager, chatCommands, roomsManager, database, accountInfo, response))
			{
				return;
			}

			const char* senderNickname = accountInfo.nickname;
			const char* message = reinterpret_cast<const char*>(request.getData());
			std::vector<std::uint8_t> responseData(playerNameLength + request.getOption());
			std::copy(senderNickname, senderNickname + playerNameLength, responseData.begin());
			std::copy(message, message + request.getOption(), responseData.begin() + playerNameLength);
			response.setData(responseData.data(), responseData.size());

			if (request.getExtra() != Enums::ChatExtra::CLAN)
			{
				if (Main::Classes::Room* room = roomsManager.getRoomByNumber(session.getRoomNumber()))
				{
					if (room->isMuted() && session.getAccountInfo().playerGrade < Common::Enums::GRADE_MOD)
					{
						Details::sendMessage("the room is currently muted", session);
					}
					else if (session.isInMatch())
					{
						roomsManager.broadcastToMatchExceptSelf(session.getRoomNumber(), session.getAccountInfo().uniqueId, response, request.getExtra());
					}
					else
					{
						roomsManager.broadcastOutsideMatchExceptSelf(session.getRoomNumber(), session.getAccountInfo().uniqueId, response, request.getExtra());
					}
				}
			}
			else
			{
				sessionsManager.broadcastToClan(session.getId(), response);
			}
		}
	}
}

#endif
