#ifndef ELIMINATION_NEXT_ROUND_HANDLER_H
#define ELIMINATION_NEXT_ROUND_HANDLER_H

#include "../../Network/MainSession.h"
#include "../../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "Network/Packet.h"
#include "../../Classes/RoomsManager.h"
#include "../../Structures/EndScoreboard.h"

namespace Main
{
	namespace Handlers
	{
		inline void handleEliminationNextRound(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager)
		{
			roomsManager.broadcastToRoomExceptSelf(session.getRoomNumber(), session.getAccountInfo().uniqueId, const_cast<Common::Network::Packet&>(request));
		}

		inline void handleEliminationNextRound2(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager)
		{
			Common::Network::Packet response = request;
			response.setExtra(1);
			roomsManager.broadcastToRoomExceptSelf(session.getRoomNumber(), session.getAccountInfo().uniqueId, response);
		}

		inline void handleMatchEnding(const Common::Network::Packet& request, Main::Network::Session& session, Main::Classes::RoomsManager& roomsManager)
		{
			struct ClientEndingMatchNotificationHeader
			{
				std::uint32_t redScore : 8 = 0;
				std::uint32_t blueScore : 8 = 0;
				std::uint32_t unknown1 : 8 = 0; // Seems related to num of players
				std::uint32_t unknown2 : 8 = 0; // Seems related to num of players
			} clientEndMatchNotificationHeader;

			std::memcpy(&clientEndMatchNotificationHeader, request.getData(), sizeof(clientEndMatchNotificationHeader));

			// First, client sends all info of all players in the room, we resend it back to everyone
			roomsManager.broadcastToRoomExceptSelf(session.getRoomNumber(), session.getAccountInfo().uniqueId, const_cast<Common::Network::Packet&>(request));

			// Next, specific ending info for each session
			if (Main::Classes::Room* room = roomsManager.getRoomByNumber(session.getRoomNumber()))
			{
				const Main::ConstantDatabase::CdbUtil cdbUtil;
				const auto rewardInfo = cdbUtil.getRewardInfoForMode(room->getRoomSettings().mode);
				if (!rewardInfo.has_value())
				{
					return;
				}

				Common::Network::Packet response;
				for (std::size_t i = 0; i < request.getOption(); ++i)
				{
					// TODO: Add matchStartTime to Room class, and here check whether at least 60 seconds passed. Otherwise don't give anything to the players! (FARM prevention)
					Main::Structures::ClientEndingMatchNotification finalScoreGivenByClient;
					std::memcpy(&finalScoreGivenByClient, request.getData()
						+ sizeof(finalScoreGivenByClient) * i + sizeof(ClientEndingMatchNotificationHeader), sizeof(finalScoreGivenByClient));
					finalScoreGivenByClient.uniqueId.server = 4;
					Main::Structures::ScoreboardResponse responseStruct(finalScoreGivenByClient);

					auto* targetSession = room->getPlayer(finalScoreGivenByClient.uniqueId);
					if (!targetSession) continue;
					response.setTcpHeader(targetSession->getId(), Common::Enums::USER_LARGE_ENCRYPTION);
					std::cout << "EndMatch: Taking care of SessionID: " << targetSession->getId() << '\n';

					// Calculate new EXP
					auto targetAccountInfo = room->getAccountInfoFor(finalScoreGivenByClient.uniqueId);
					responseStruct.newTotalEXP = targetAccountInfo.experience
						+ rewardInfo->ri_exp_base + rewardInfo->ri_exp_kill * responseStruct.totalKills * 2 + rewardInfo->ri_exp_death * responseStruct.deaths
						+ rewardInfo->ri_exp_assist * responseStruct.assists;

					// Calculate new MP
					responseStruct.newTotalMP = targetAccountInfo.microPoints
						+ rewardInfo->ri_poi_base + rewardInfo->ri_poi_kill * responseStruct.totalKills * 2 + rewardInfo->ri_poi_death * responseStruct.deaths
						+ rewardInfo->ri_poi_assist * responseStruct.assists;

					// Check if player leveled up
					auto gradeInfo = cdbUtil.getGradeInfoForLevel(targetAccountInfo.playerLevel + 1);
					if (gradeInfo != std::nullopt)
					{
						if (responseStruct.newTotalEXP >= gradeInfo->gi_exp)
						{
							responseStruct.newTotalMP += gradeInfo->gi_reward_point;
							room->storeEndMatchStatsFor(finalScoreGivenByClient.uniqueId, responseStruct,
								clientEndMatchNotificationHeader.blueScore, clientEndMatchNotificationHeader.redScore, true);

							const auto newPlayerLevel = targetAccountInfo.playerLevel + 1;
							response.setCommand(311, 0, 1, newPlayerLevel); // level up packet
							response.setData(reinterpret_cast<std::uint8_t*>(&finalScoreGivenByClient.uniqueId), sizeof(finalScoreGivenByClient.uniqueId));
							room->broadcastToRoomExceptSelf(response, finalScoreGivenByClient.uniqueId);

							if (targetSession)
							{ 
								// todo: spawn won item
								// Give MP proportionally to the new level
								targetSession->setAccountMicroPoints(targetAccountInfo.microPoints + (newPlayerLevel * 700));
							}
						}
						else
						{
							room->storeEndMatchStatsFor(finalScoreGivenByClient.uniqueId, responseStruct,
								clientEndMatchNotificationHeader.blueScore, clientEndMatchNotificationHeader.redScore, false);
						}
					}
					else
					{
						room->storeEndMatchStatsFor(finalScoreGivenByClient.uniqueId, responseStruct,
							clientEndMatchNotificationHeader.blueScore, clientEndMatchNotificationHeader.redScore, false);
					}

					// TODO: THE SIZE OF THIS PACKET CAN BE >= 1440 BYTES IF THERE ARE MAX NUM OF PLAYERS + MAX NUM OF OBS PLAYERS!
					// BUT ARE OBS PLAYERS EVEN COUNTED???
					response.setCommand(request.getOrder(), 0, 1, 0);
					response.setData(reinterpret_cast<std::uint8_t*>(&responseStruct), sizeof(responseStruct));
					room->sendTo(finalScoreGivenByClient.uniqueId, response);
				}

				room->endMatch();

				// Notify client about match leave, MVR does this as well
				response.setCommand(256, 0, 33, 0);
				response.setData(nullptr, 0);
				session.asyncWrite(response);
			}
		}
	}
}

#endif
