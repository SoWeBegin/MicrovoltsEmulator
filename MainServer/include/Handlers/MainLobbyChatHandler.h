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
			if (playerGrade == PlayerGrade::GRADE_NORMAL || playerGrade == PlayerGrade::GRADE_MOD) return static_cast<ChatGrade>(playerGrade - 2);
			if (playerGrade == PlayerGrade::GRADE_TESTER) return ChatGrade::CHAT_TESTER;
			return ChatGrade::CHAT_GM;
		}

		inline void handleLobbyChatMessage(const Common::Network::Packet& request, Main::Network::Session& session, Main::Network::SessionsManager& sessionsManager,
			Main::Command::ChatCommands& chatCommands, Main::Classes::RoomsManager& roomsManager, Main::Persistence::PersistentDatabase& database)
		{

			Common::Network::Packet response;
			response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);

			if (session.isMuted())
			{
				std::string m_confirmationMessage{ std::string(16, '0') };
				m_confirmationMessage += "you have been muted by a moderator.";
				response.setOrder(316);
				response.setExtra(1);
				response.setData(reinterpret_cast<std::uint8_t*>(m_confirmationMessage.data()), m_confirmationMessage.size());
				response.setOption(m_confirmationMessage.size());
				response.setMission(0);
				session.asyncWrite(response);
				return;
			}
			
			response.setOrder(316);
			response.setOption(request.getOption()); 
			response.setExtra(request.getExtra()); 

			const auto& accountInfo = session.getAccountInfo();
			response.setMission(getChatGrade(static_cast<Common::Enums::PlayerGrade>(accountInfo.playerGrade)));

			if (request.getExtra() == Enums::ChatExtra::COMMAND) 
			{
				std::string command{ reinterpret_cast<const char*>(request.getData() + 1), static_cast<std::size_t>(request.getOption() - 1)};
				if (command == "?")
				{
					chatCommands.showUsages(session, response, static_cast<Common::Enums::PlayerGrade>(accountInfo.playerGrade));
				}
				std::string commandName = command.substr(0, command.find(' '));
				if (!chatCommands.executeSimpleCommand(commandName, session, sessionsManager, response))
				{
					if (!chatCommands.executeComplexCommand(commandName, command, session, sessionsManager, response))
					{
						if (!chatCommands.executeDatabaseCommand(commandName, command, session, database, response))
						{
							auto roomNumber = session.getRoomNumber();
							if (!roomNumber) return;
							if (!chatCommands.executeSimpleRoomCommand(commandName, session, roomsManager, roomNumber, response))
							{
								if (!chatCommands.executeComplexRoomCommand(commandName, command, session, roomsManager, response))
								{
									std::cout << "Inside if statement\n";
								}
							}
						}
					}
				}
			}

			constexpr std::size_t playerNameLength = 16;
			const char* senderNickname = accountInfo.nickname;
			if (request.getExtra() == Enums::ChatExtra::WHISPER) 
			{
				constexpr std::size_t uidSize = sizeof(accountInfo.uniqueId);
				const uint8_t* uid = reinterpret_cast<const uint8_t*>(&(accountInfo.uniqueId));
				const char* message = reinterpret_cast<const char*>(request.getData() + playerNameLength);
				std::vector<std::uint8_t> responseData(playerNameLength + request.getOption() + uidSize);
				std::copy(uid, uid + uidSize, responseData.begin());
				std::copy(senderNickname, senderNickname + playerNameLength, responseData.begin() + uidSize);
				std::copy(message, message + request.getOption(), responseData.begin() + playerNameLength + uidSize);
				response.setData(responseData.data(), responseData.size());

				char receiverNickname[16];
				std::memcpy(receiverNickname, request.getData(), playerNameLength);
				
				auto* targetSession = sessionsManager.findSessionByName(receiverNickname);
				if (targetSession)
				{
					// Warning: This will NOT work with multithreading: targetSession might still become invalidated here
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
				response.setMission(1); // 1 = WhisperConfirmation
				response.setData(nullptr, 0);
				session.asyncWrite(response);
			}

			// Todo: Add team chat 
			else if (request.getExtra() == Enums::ChatExtra::NORMAL || request.getExtra() == Enums::ChatExtra::CLAN
				|| request.getExtra() == Enums::ChatExtra::TEAM)  // yes, these conditions ARE necessary
			{
				const char* message = reinterpret_cast<const char*>(request.getData());
				std::vector<std::uint8_t> responseData(playerNameLength + request.getOption());
				std::copy(senderNickname, senderNickname + playerNameLength, responseData.begin()); 
				std::copy(message, message + request.getOption(), responseData.begin() + playerNameLength);
				response.setData(responseData.data(), responseData.size());

				if (request.getOrder() == 161)
				{
					auto room = roomsManager.getRoomByNumber(session.getRoomNumber());
					if (!room) return;
					auto& actualRoom = room->get();

					if (actualRoom.isMuted() && session.getAccountInfo().playerGrade < Common::Enums::GRADE_MOD)
					{
						std::string m_confirmationMessage{ std::string(16, '0') };
						m_confirmationMessage += "the room is currently muted";
						response.setOrder(316);
						response.setExtra(1);
						response.setData(reinterpret_cast<std::uint8_t*>(m_confirmationMessage.data()), m_confirmationMessage.size());
						response.setOption(m_confirmationMessage.size());
						response.setMission(0);
						session.asyncWrite(response);
					}
					else
					{
						if (session.isInMatch())
						{
							roomsManager.broadcastToMatchExceptSelf(session.getRoomNumber(), session.getAccountInfo().uniqueId, response, request.getExtra());
						}
						else
						{
							roomsManager.broadcastOutsideMatchExceptSelf(session.getRoomNumber(), session.getAccountInfo().uniqueId, response, request.getExtra());
						}
						return;
					}
				}
				if (request.getExtra() == Enums::ChatExtra::NORMAL) sessionsManager.broadcastToLobbyExceptSelf(session.getId(), response);
				else sessionsManager.broadcastToClan(session.getId(), response);
			}
		}
	}
}

#endif