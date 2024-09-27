
#include <cstdint>
#include <vector>
#include <string>
#include "../../include/Structures/Room/ClientRoomCreationInfo.h"
#include "../../include/Structures/Room/RoomPlayerInfo.h"
#include "../../include/Structures/Room/RoomsList.h"
#include "../../include/Structures/Room/RoomJoin.h"
#include "../../include/Structures/Room/RoomPlayerItems.h"
#include "../../include/Structures/Room/RoomPlayerClan.h"
#include "../../include/Structures/Room/RoomSettingsUpdate.h"
#include "../../include/Classes/Player.h"
#include "../../include/Network/MainSession.h"
#include "../../include/Structures/Room/RoomPlayerInfo.h"
#include "../../include/Classes/Room.h"
#include "../../include/Handlers/Room/RoomLeaveHandler.h"
#include <Enums/PlayerEnums.h>
#include <Utils/Logger.h>
#include <range/v3/all.hpp>
#include "../../include/Utilities.h"

namespace Main
{
	namespace Classes
	{
		Room::Room(const std::string& title, const Main::Structures::RoomSettings& settings, const Main::Structures::RoomPlayerInfo& player, Main::Network::Session* session)
			: m_title{ title }, m_settings{ settings }, m_number{ ++idCounter }
		{
			const auto pair = std::pair{ player, session };
			m_players.push_back(pair);
			m_playerSessionIdToVecIdx[session->getId()] = m_players.size() - 1;
			m_isTeamBalanceOn = false; // For now, team balance isn't supported as it causes issues e.g. team bug.
		}

		Main::Structures::RoomPlayerInfo Room::createRoomPlayerInfo(Main::Network::Session* session, std::uint32_t team) const
		{
			const auto& accountInfo = session->getAccountInfo();
			Main::Structures::RoomPlayerInfo roomPlayerInfo;
			roomPlayerInfo.character = accountInfo.latestSelectedCharacter;
			roomPlayerInfo.level = accountInfo.playerLevel;
			roomPlayerInfo.ping = session->getPing();
			std::memcpy(roomPlayerInfo.playerName, accountInfo.nickname, 16);
			roomPlayerInfo.uniqueId = accountInfo.uniqueId;
			roomPlayerInfo.team = team;
			roomPlayerInfo.state = Common::Enums::STATE_WAITING;
			return roomPlayerInfo;
		}

		void Room::addPlayer(Main::Network::Session* session, std::uint32_t team)
		{
			auto roomPlayerInfo = createRoomPlayerInfo(session, team);
			m_players.emplace_back(roomPlayerInfo, session);
			m_playerSessionIdToVecIdx[session->getId()] = m_players.size() - 1;
			session->setRoomNumber(m_number);
		}

		void Room::addObserverPlayer(Main::Network::Session* session)
		{
			auto roomPlayerInfo = createRoomPlayerInfo(session, Common::Enums::Team::TEAM_OBSERVER);
			m_observerPlayers.emplace_back(roomPlayerInfo, session);
			m_obsPlayerSessionIdToVecIdx[session->getId()] = m_observerPlayers.size() - 1;
			session->setRoomNumber(m_number);
		}

		std::uint32_t Room::getPlayerIdx(std::uint64_t playerId) const
		{
			auto indexIter = m_playerSessionIdToVecIdx.find(playerId);
			if (indexIter != m_playerSessionIdToVecIdx.end())
			{
				return indexIter->second;
			}
			return -1;
		}

		void Room::setPassword(const std::string& password)
		{
			m_password = password;
		}

		const std::string& Room::getPassword() const
		{
			return m_password;
		}

		std::uint16_t Room::getRoomNumber() const
		{
			return m_number;
		}

		void Room::updatePlayerInfo(Main::Network::Session* session) 
		{
			auto indexIter = m_playerSessionIdToVecIdx.find(session->getId());
			if (indexIter != m_playerSessionIdToVecIdx.end())
			{
				const auto& accountInfo = session->getAccountInfo();
				m_players[indexIter->second].first.character = accountInfo.latestSelectedCharacter;
				m_players[indexIter->second].first.level = accountInfo.playerLevel;
			}
		}

		std::size_t Room::getPlayersCount() const
		{
			return m_players.size();
		}

		void Room::addKickedPlayer(std::uint32_t accountId)
		{
			m_kickedPlayerAccountIds.push_back(accountId);
		}

		void Room::muteRoom()
		{
			m_isMuted = true;
		}

		void Room::unmuteRoom()
		{
			m_isMuted = false;
		}

		bool Room::isMuted() const
		{
			return m_isMuted;
		}

		void Room::removeAllPlayers()
		{
			Common::Network::Packet removePlayerFromRoomServerRequest;
			removePlayerFromRoomServerRequest.setOrder(137);
			removePlayerFromRoomServerRequest.setExtra(1);

			auto removePlayer = [&](auto& players) 
				{
				for (auto& player : players)
				{
					removePlayerFromRoomServerRequest.setTcpHeader(player.second->getId(), Common::Enums::USER_LARGE_ENCRYPTION);
					player.second->asyncWrite(removePlayerFromRoomServerRequest);
					player.second->leaveRoom();
				}
				players.clear();
				};

			removePlayer(m_players);
			removePlayer(m_observerPlayers);
			m_obsPlayerSessionIdToVecIdx.clear();
			m_playerSessionIdToVecIdx.clear();
		}

		// Returns the index of the player with the best ms, and if no player is found that meets the requirements, -1 is returned to signal that.
		std::uint32_t Room::getBestMsIndexExceptSelf(bool checkIsInMatch, std::uint64_t selfId)
		{
			std::uint32_t bestMs = std::numeric_limits<std::uint32_t>::max();
			std::uint32_t bestMsPlayerIdx = 0;
			std::uint32_t currentIdx = 0;
			bool foundBestMs = false;

			for (const auto& [roomInfo, session] : m_players)
			{
				if (selfId != session->getId() && session->getPing() < bestMs)
				{
					if (checkIsInMatch && session->isInMatch())
					{
						bestMs = session->getPing();
						bestMsPlayerIdx = currentIdx;
						foundBestMs = true;
					}
					else if (!checkIsInMatch)
					{
						bestMs = session->getPing();
						bestMsPlayerIdx = currentIdx;
						foundBestMs = true;
					}
				}
				++currentIdx;
			}
			return foundBestMs ? bestMsPlayerIdx : -1;
		}

		// Returns true if the room has to also be closed, false otherwise
		bool Room::removeHostFromMatch()
		{
			Utils::Logger& logger = Utils::Logger::getInstance();

			Main::Structures::UniqueId originalHostUniqueId = m_players[0].second->getAccountInfo().uniqueId;
			const std::uint64_t originalHostSessionId = m_players[0].second->getId();

			Common::Network::Packet removePlayerFromRoomServerRequest;
			removePlayerFromRoomServerRequest.setTcpHeader(originalHostSessionId, Common::Enums::USER_LARGE_ENCRYPTION);
			removePlayerFromRoomServerRequest.setOrder(137);
			removePlayerFromRoomServerRequest.setExtra(1);

			Common::Network::Packet removePlayerFromMatchServerRequest;
			removePlayerFromMatchServerRequest.setTcpHeader(originalHostSessionId, Common::Enums::USER_LARGE_ENCRYPTION);
			removePlayerFromMatchServerRequest.setOrder(256);
			removePlayerFromMatchServerRequest.setData(reinterpret_cast<std::uint8_t*>(&originalHostUniqueId), sizeof(originalHostUniqueId));

			Common::Network::Packet hostChange;
			hostChange.setTcpHeader(originalHostSessionId, Common::Enums::USER_LARGE_ENCRYPTION);
			hostChange.setOrder(124);

			// Signal that the host has been removed from the match
			m_players[0].second->setIsInMatch(false);

			// Count total players inside the match and total players inside the room
			const std::size_t totalPlayersInMatch = std::count_if(m_players.begin(), m_players.end(), [](const auto& currentPlayer) { return currentPlayer.second->isInMatch(); });
			const std::uint32_t totalPlayersInRoom = m_players.size();

			// Check if there are players inside the match that can be changed to the (match) host. 
			// We use >= 1 because the host has been removed from the match above: there might be only one single player in the match, e.g. if this is square mode!
			if (totalPlayersInMatch >= 1)
			{
				logger.log("There are still some players in the match left, after the host left the match. Attempting to pick up new host...",
					Utils::LogType::Normal, "Room::removeHostFromMatch");

				// Find the player with best ms inside the match
				const std::uint32_t bestMsPlayerIdxInMatch = getBestMsIndexExceptSelf(true, originalHostSessionId);
				if (changeHost(bestMsPlayerIdxInMatch))
				{
					logger.log("The host has been changed from player " + std::string(m_players[bestMsPlayerIdxInMatch].second->getPlayerName())
						+ " to player: " + std::string(m_players[0].second->getPlayerName())
						+ ", idx of new host: " + std::to_string(bestMsPlayerIdxInMatch),
						Utils::LogType::Normal, "Room::removeHostFromMatch");

					hostChange.setOption(bestMsPlayerIdxInMatch);
					hostChange.setExtra(Common::Enums::CHANGE_HOST_SUCCESS);
					broadcastToRoom(hostChange);

					// Unmap original host
					m_playerSessionIdToVecIdx.erase(originalHostSessionId);

					// Leave match + room packets.
					m_players[bestMsPlayerIdxInMatch].second->asyncWrite(removePlayerFromRoomServerRequest);
					m_players[bestMsPlayerIdxInMatch].second->leaveRoom();
					broadcastToRoomExceptSelf(removePlayerFromMatchServerRequest, originalHostUniqueId);
					// NOTE: This decrements all player indexes inside the client! Call it ONLY AFTER sending the packet to change the host!
					Main::Handlers::notifyRoomPlayerLeaves(originalHostUniqueId, *this);

					logger.log("Removed previous host from room. " + m_players[bestMsPlayerIdxInMatch].second->getPlayerInfoAsString(),
						Utils::LogType::Normal, "Room::removeHostFromMatch");

					m_players.erase(m_players.begin() + bestMsPlayerIdxInMatch);
					return false;
				}
				else
				{
					// Something went wrong: There were still some players in the match, but none of them could be picked up as the new host.
					// This can be caused b the "IsInMatch" flag being set incorrectly for some players. The server may think no players are in the match
					// even if they are. In this case we cannot safely proceed further: Just notify the players about the error and close the room.
					hostChange.setExtra(Common::Enums::CHANGE_HOST_FAIL); // "Unknown error. Please restart the client" 
					broadcastToRoom(hostChange); 

					logger.log("The host could not be switched to a new one! Closing the room to avoid further issues.",
						Utils::LogType::Error, "Room::removeHostFromMatch");

					return true; // Close the room as we cannot proceed further
				}
			}

			// Otherwise, check if there are players inside the room that can be changed to the actual room host.
			// We use >1 and not >= 1 because the host is still counted in the room. 
			else if (totalPlayersInRoom > 1)
			{
				logger.log("The host is leaving and there are no players in the match. However, there are players in the room. Attempting to pick up new host...",
					Utils::LogType::Normal, "Room::removeHostFromMatch");

				const std::uint32_t bestMsIndexPlayerOutsideMatch = getBestMsIndexExceptSelf(false, originalHostSessionId);
				if (changeHost(bestMsIndexPlayerOutsideMatch))
				{
					logger.log("The host has been changed from player " + std::string(m_players[bestMsIndexPlayerOutsideMatch].second->getPlayerName())
						+ " to player: " + std::string(m_players[0].second->getPlayerName())
						+ ", idx of new host: " + std::to_string(bestMsIndexPlayerOutsideMatch),
						Utils::LogType::Normal, "Room::removeHostFromMatch");

					// Unmap original host
					m_playerSessionIdToVecIdx.erase(originalHostSessionId);

					// If a new match-host has been taken, notify whole room about it.
					hostChange.setOption(bestMsIndexPlayerOutsideMatch);
					hostChange.setExtra(Common::Enums::CHANGE_HOST_SUCCESS);
					broadcastToRoom(hostChange);

					// The previous host leaves the match, host->setIsInMatch(false) already set at this point
					m_players[bestMsIndexPlayerOutsideMatch].second->asyncWrite(removePlayerFromRoomServerRequest);
					m_players[bestMsIndexPlayerOutsideMatch].second->leaveRoom();
					broadcastToRoomExceptSelf(removePlayerFromMatchServerRequest, originalHostUniqueId);
					// NOTE: This decrements all player indexes inside the client! Call it ONLY AFTER sending the packet to change the host!
					Main::Handlers::notifyRoomPlayerLeaves(originalHostUniqueId, *this);

					logger.log("Removed previous host from room. " + m_players[bestMsIndexPlayerOutsideMatch].second->getPlayerInfoAsString(),
						Utils::LogType::Normal, "Room::removeHostFromMatch");

					m_players.erase(m_players.begin() + bestMsIndexPlayerOutsideMatch);

					m_hasMatchStarted = false; // The host was the only one inside the match, so we end it
					return false; // Another player got the host in the room, no need to close it
				}
				else
				{
					// Something went wrong: There were still some players in the room, but none of them could be picked up as the new host.
					// This could be caused by adding and removing players from the room incorrectly.
					// In this case we cannot safely proceed further: Just notify the players about the error and close the room.
					hostChange.setExtra(Common::Enums::CHANGE_HOST_FAIL);
					broadcastToRoom(hostChange); // "Unknown error. Please restart the client" 

					logger.log("The host could not be switched to a new one! Closing the room to avoid further issues.",
						Utils::LogType::Error, "Room::removeHostFromMatch");

					return true;
				}
			}
		}

		// Returns true if the room must be also be closed (e.g. due to host-switch errors, or because no other player is inside the room), false otherwise.
		bool Room::removePlayer(Main::Network::Session* session, std::uint32_t extra)
		{
			Utils::Logger& logger = Utils::Logger::getInstance();

			Common::Network::Packet removePlayerFromRoomServerRequest;
			removePlayerFromRoomServerRequest.setTcpHeader(session->getId(), Common::Enums::USER_LARGE_ENCRYPTION);
			removePlayerFromRoomServerRequest.setOrder(137);
			removePlayerFromRoomServerRequest.setExtra(extra);
			std::uint32_t originalSessionId = session->getId();

			auto playerIter = std::find_if(m_players.begin(), m_players.end(), [session](const auto& currentSession) { return currentSession.second == session; });

			// we're taking care of a non-observer player
			if (playerIter != m_players.end())
			{
				const std::uint32_t bestMsIndexPlayerOutsideMatch = getBestMsIndexExceptSelf(false, playerIter->second->getId());

				// The player to be removed is the host. Find a new host.
				if (playerIter->second->getId() == m_players[0].second->getId())
				{
					Main::Structures::UniqueId uniqueId = playerIter->second->getAccountInfo().uniqueId;
					logger.log("The player " + playerIter->second->getPlayerInfoAsString() + " is host and is attempting to leave their room", Utils::LogType::Normal, "Room::removePlayer");

					// if host is alone in room, getBestMsIndexExceptSelf returns -1, and changeHost will return false.
					if (changeHost(bestMsIndexPlayerOutsideMatch))
					{
						logger.log("The host has been changed from player " + std::string(m_players[bestMsIndexPlayerOutsideMatch].second->getPlayerName())
							+ " to player: " + std::string(m_players[0].second->getPlayerName())
							+ ", idx of new host: " + std::to_string(bestMsIndexPlayerOutsideMatch),
							Utils::LogType::Normal, "Room::removePlayer");

						// Notify room about the new host.
						Common::Network::Packet hostChange;
						hostChange.setTcpHeader(playerIter->second->getId(), Common::Enums::USER_LARGE_ENCRYPTION);
						hostChange.setOrder(124);
						hostChange.setOption(bestMsIndexPlayerOutsideMatch);
						hostChange.setExtra(Common::Enums::CHANGE_HOST_SUCCESS);
						broadcastToRoom(hostChange);

						// At this point playerIter is invalidated, update it
						playerIter = std::find_if(m_players.begin(), m_players.end(), [session](const auto& currentSession) { return currentSession.second == session; });
					}
					else if (m_players.size() > 1) // Host not changed, even if it should have (because there were other players apart from the host inside the room)
					{
						// Notify whoever tried to switch host that it cannot be done
						Common::Network::Packet hostChange;
						hostChange.setTcpHeader(playerIter->second->getId(), Common::Enums::USER_LARGE_ENCRYPTION);
						hostChange.setOrder(124);
						hostChange.setExtra(Common::Enums::CHANGE_HOST_FAIL);
						broadcastToRoom(hostChange); // "Unknown error. Please restart the client" 

						logger.log("The host could not be switched to a new one! Closing the room to avoid further issues.",
							Utils::LogType::Error, "Room::removePlayer");

						return true; // Something went wrong while trying to switch to a new host after the previous one left the room.
						// In this case, just close the room to avoid further issues.
					}
				}

				// Remove the player from the room
				playerIter->second->asyncWrite(removePlayerFromRoomServerRequest);
				playerIter->second->leaveRoom();		
				// NOTE: This decrements all player indexes in the client! Do it always after sending the change host packet!
				Main::Handlers::notifyRoomPlayerLeaves(playerIter->second->getAccountInfo().uniqueId, *this);
				m_players.erase(playerIter);

				// unmap original host
				m_playerSessionIdToVecIdx.erase(originalSessionId);

				logger.log("Removed player from room. " + playerIter->second->getPlayerInfoAsString(),
					Utils::LogType::Normal, "Room::removePlayer");
				logger.log(getRoomInfoAsString(), Utils::LogType::Normal, "Room::removePlayer");

				return m_players.empty(); // If empty, the room must be also closed.
			}

			// otherwise, we're taking care of a player that is in observer mode. Check if that's the case and whether it actually exists in m_observerPlayers.
			auto observerIter = std::find_if(m_observerPlayers.begin(), m_observerPlayers.end(), [session](const auto& currentSession) { return currentSession.second == session; });
			if (observerIter != m_observerPlayers.end())
			{
				logger.log("Removed observer player from room. " + observerIter->second->getPlayerInfoAsString(),
					Utils::LogType::Normal, "Room::removePlayer");

				observerIter->second->asyncWrite(removePlayerFromRoomServerRequest);
				observerIter->second->leaveRoom();
				Main::Handlers::notifyRoomPlayerLeaves(observerIter->second->getAccountInfo().uniqueId, *this);
				m_observerPlayers.erase(observerIter);
				m_obsPlayerSessionIdToVecIdx.erase(originalSessionId);
			}
			return false;
		}

		std::vector<Main::Structures::RoomPlayerInfo> Room::getAllPlayers() const
		{
			std::vector<Main::Structures::RoomPlayerInfo> allPlayers;
			allPlayers.reserve(m_players.size() + m_observerPlayers.size());

			std::transform(m_players.begin(), m_players.end(), std::back_inserter(allPlayers),
				[](const std::pair<Main::Structures::RoomPlayerInfo, Main::Network::Session*>& pair) {
					return pair.first;
				});

			std::transform(m_observerPlayers.begin(), m_observerPlayers.end(), std::back_inserter(allPlayers),
				[](const std::pair<Main::Structures::RoomPlayerInfo, Main::Network::Session*>& pair) {
					return pair.first;
				});

			return allPlayers;
		}

		Main::Structures::SingleRoom Room::getRoomInfo() const
		{
			Main::Structures::SingleRoom roomInfo;
			roomInfo.hasPassword = !m_password.empty();
			roomInfo.isObserverOff = m_settings.isObserverModeOn ? 0 : 1;
			roomInfo.map = m_settings.map;
			roomInfo.matchStarted = m_hasMatchStarted;
			roomInfo.maxPlayers = m_settings.playersPerTeam ? m_settings.playersPerTeam * 2 : 1;
			roomInfo.mode = m_settings.mode;
			roomInfo.numPlayers = m_players.size();
			roomInfo.roomNumber = m_number - 1;
			m_title.copy(roomInfo.title, sizeof(roomInfo.title) - 1);
			roomInfo.title[m_title.size()] = '\0';
			roomInfo.weaponRestriction = m_settings.weaponRestriction;
			roomInfo.ping = m_players[0].second->getPing();
			return roomInfo;
		}

		const Main::Structures::RoomSettings& Room::getRoomSettings() const
		{
			return m_settings;
		}

		Main::Structures::RoomJoin Room::getRoomJoinInfo() const
		{
			Main::Structures::RoomJoin roomJoinInfo;
			roomJoinInfo.hasPassword = !m_password.empty();
			roomJoinInfo.isObserverOn = m_settings.isObserverModeOn;
			roomJoinInfo.isOpen = m_settings.isOpen;
			roomJoinInfo.map = m_settings.map;
			roomJoinInfo.mode = m_settings.mode;
			roomJoinInfo.hidePassword = false; 
			roomJoinInfo.maxPlayers = m_settings.playersPerTeam == 0 ? 1 : m_settings.playersPerTeam * 2;
			roomJoinInfo.weaponRestriction = m_settings.weaponRestriction;
			roomJoinInfo.isTeamBalanceOn = m_isTeamBalanceOn;
			roomJoinInfo.hasMatchStarted = m_hasMatchStarted;
			std::memcpy(roomJoinInfo.password, m_password.data(), 9);
			return roomJoinInfo;
		}

		std::vector<Main::Structures::RoomPlayerItems> Room::getPlayersItems() const
		{
			std::vector<Main::Structures::RoomPlayerItems> ret;
			for (const auto& [roomInfo, session] : ranges::views::concat(m_players, m_observerPlayers))
			{
				Main::Structures::RoomPlayerItems roomPlayerItems;
				const auto separatedItems = session->getEquippedItemsSeparated(); 
				roomPlayerItems.equippedItems = separatedItems.first;
				roomPlayerItems.equippedWeapons = separatedItems.second;
				roomPlayerItems.uniqueId = roomInfo.uniqueId;
				ret.push_back(roomPlayerItems);
			}
			return ret;
		}

		std::vector<Main::Structures::PlayerClan> Room::getPlayersClans() const
		{
			std::vector<Main::Structures::PlayerClan> ret;
			for (const auto& [roomInfo, session] : ranges::views::concat(m_players, m_observerPlayers))
			{
				const auto& accountInfo = session->getAccountInfo();
				Main::Structures::PlayerClan playerClan;
				if (accountInfo.clanId > 8)
				{
					playerClan.clanLogoBackId = accountInfo.clanLogoBackId;
					playerClan.clanLogoFrontId = accountInfo.clanLogoFrontId;
					std::memcpy(playerClan.clanName, accountInfo.clanName, 16);
					playerClan.unknown2 = accountInfo.clanId;
				}
				else
				{
					playerClan.handlePlayerInvite = 1;
				}
				ret.push_back(playerClan);
			}
			return ret;
		}

		void Room::breakroom()
		{
			for (auto& [unused, session] : ranges::views::concat(m_players, m_observerPlayers))
			{
				session->leaveRoom();
			}
			m_players.clear();
			m_observerPlayers.clear();
			m_obsPlayerSessionIdToVecIdx.clear();
			m_playerSessionIdToVecIdx.clear();
		}

		// Broadcasts to whole room (room + match)
		void Room::broadcastToRoom(Common::Network::Packet& packet)
		{
			for (auto& [roomInfo, session] : ranges::views::concat(m_players, m_observerPlayers))
			{
				packet.setTcpHeader(session->getSessionId(), Common::Enums::USER_LARGE_ENCRYPTION);
				session->asyncWrite(packet);
			}
		}

		void Room::broadcastToRoomExceptSelf(Common::Network::Packet& packet, const Main::Structures::UniqueId& uniqueId)
		{
			for (auto& currentPlayer : ranges::views::concat(m_players, m_observerPlayers))
			{
				if (currentPlayer.first.uniqueId == uniqueId) continue;
				packet.setTcpHeader(currentPlayer.second->getSessionId(), Common::Enums::USER_LARGE_ENCRYPTION);
				currentPlayer.second->asyncWrite(packet);
			}
		}

		void Room::broadcastToTeamExceptSelf(Common::Network::Packet& packet, const Main::Structures::UniqueId& uniqueId, bool checkInsideMatch)
		{
			auto indexIter = m_playerSessionIdToVecIdx.find(uniqueId.session);
			if (indexIter != m_playerSessionIdToVecIdx.end())
			{
				Common::Enums::Team targetTeam = static_cast<Common::Enums::Team>(m_players[indexIter->second].first.team);
				for (auto& [roomInfo, session] : m_players)
				{
					if (roomInfo.uniqueId != uniqueId && roomInfo.team == targetTeam && ((checkInsideMatch && session->isInMatch()) || (!checkInsideMatch && !session->isInMatch())))
					{
						packet.setTcpHeader(session->getSessionId(), Common::Enums::USER_LARGE_ENCRYPTION);
						session->asyncWrite(packet);
					}
				}
			}
		}

		void Room::broadcastToMatchExceptSelf(Common::Network::Packet& packet, const Main::Structures::UniqueId& uniqueId, std::uint32_t extra)
		{
			if (extra == Main::Enums::ChatExtra::TEAM)
			{
				broadcastToTeamExceptSelf(packet, uniqueId, true);
			}
			else
			{
				bool broadcastOnlyToObs = false;
				auto indexIter = m_obsPlayerSessionIdToVecIdx.find(uniqueId.session);
				if (indexIter != m_obsPlayerSessionIdToVecIdx.end())
				{
					// >= MOD grade messages are broadcast to whole room, even outside observer
					if (m_observerPlayers[indexIter->second].second->getAccountInfo().playerGrade < Common::Enums::GRADE_MOD) 
					{
						broadcastOnlyToObs = true;
					}
				}
				for (auto& [roomInfo, session] : m_observerPlayers)
				{
					if (roomInfo.uniqueId == uniqueId) continue;
					if (session->isInMatch())
					{
						packet.setTcpHeader(session->getSessionId(), Common::Enums::USER_LARGE_ENCRYPTION);
						session->asyncWrite(packet);
					}
				}
				if (!broadcastOnlyToObs)
				{
					for (auto& [roomInfo, session]: m_players)
					{
						if (roomInfo.uniqueId != uniqueId && session->isInMatch())
						{
							packet.setTcpHeader(session->getSessionId(), Common::Enums::USER_LARGE_ENCRYPTION);
							session->asyncWrite(packet);
						}
					}
				}
			}
		}

		void Room::broadcastOutsideMatchExceptSelf(Common::Network::Packet& packet, const Main::Structures::UniqueId& uniqueId, std::uint32_t extra)
		{
			if (extra == Main::Enums::ChatExtra::TEAM)
			{
				broadcastToTeamExceptSelf(packet, uniqueId, false);
			}
			else
			{
				for (auto& [roomInfo, session] : ranges::views::concat(m_players, m_observerPlayers))
				{
					if (roomInfo.uniqueId == uniqueId || session->isInMatch()) continue;
					packet.setTcpHeader(session->getSessionId(), Common::Enums::USER_LARGE_ENCRYPTION);
					session->asyncWrite(packet);
				}
			}
		}

		bool Room::hasMatchStarted() const
		{
			return m_hasMatchStarted;
		}

		void Room::endMatch()
		{
			for (auto& [roomInfo, session] : ranges::views::concat(m_players, m_observerPlayers))
			{
				if (session->isInMatch())
				{
					roomInfo.state = Common::Enums::STATE_WAITING;
					session->setIsInMatch(false);
				}
			}
			m_hasMatchStarted = false;
		}

		// this is used by both (!) manual AND automatic host-changes
		bool Room::changeHost(std::size_t newHostIdx)
		{
			if (newHostIdx >= m_players.size())
			{
				return false;
			}
			std::swap(m_players[0], m_players[newHostIdx]); 
			m_playerSessionIdToVecIdx[m_players[0].second->getId()] = 0; // map the new host
			m_playerSessionIdToVecIdx[m_players[newHostIdx].second->getId()] = newHostIdx;
			return true;
		}

		Main::Network::Session* Room::getPlayer(const Main::Structures::UniqueId& uniqueId)
		{
			auto indexIter = m_playerSessionIdToVecIdx.find(uniqueId.session);
			if (indexIter != m_playerSessionIdToVecIdx.end())
			{
				return m_players[indexIter->second].second;
			}
			return nullptr;
		}

		bool Room::isHost(const Main::Structures::UniqueId& uniqueId) const
		{
			return m_players[0].first.uniqueId == uniqueId;
		}

		void Room::setSpecificSetting(std::uint8_t setting)
		{
			m_specificSetting = setting;
		}

		void Room::setTime(std::uint16_t time)
		{
			m_settings.time = time;
		}

		// Returns new team for this player, to notify the client. Nullopt for failure.
		std::optional<std::uint32_t> Room::changePlayerTeam(const Main::Structures::UniqueId& uniqueId, std::uint32_t newTeam) {
			auto playerIt = m_playerSessionIdToVecIdx.find(uniqueId.session);
			if (playerIt != m_playerSessionIdToVecIdx.end()) 
			{
				auto index = playerIt->second; 
				if (newTeam == Common::Enums::TEAM_OBSERVER) 
				{
					if (m_observerPlayers.size() >= 10) return std::nullopt;
					auto playerInfo = std::move(m_players[index]);
					playerInfo.first.team = Common::Enums::TEAM_OBSERVER;
					m_observerPlayers.push_back(std::move(playerInfo));
					m_players.erase(m_players.begin() + index); 
					m_playerSessionIdToVecIdx.erase(uniqueId.session); 
					m_obsPlayerSessionIdToVecIdx[uniqueId.session] = m_observerPlayers.size() - 1;
				}
				else if (newTeam == Common::Enums::TEAM_ZOMBIE) 
				{
					m_players[index].first.team = newTeam; 
				}
				else 
				{
					m_players[index].first.team = m_players[index].first.team == Common::Enums::TEAM_BLUE ? Common::Enums::TEAM_RED : Common::Enums::TEAM_BLUE;
				}
				return newTeam;
			}

			auto observerIt = m_obsPlayerSessionIdToVecIdx.find(uniqueId.session);
			if (observerIt != m_obsPlayerSessionIdToVecIdx.end())
			{
				auto index = observerIt->second; 
				if (newTeam == Common::Enums::TEAM_ALL || newTeam == Common::Enums::TEAM_RED || newTeam == Common::Enums::TEAM_BLUE) 
				{
					if (m_players.size() >= m_settings.playersPerTeam * 2) 
					{
						return std::nullopt;
					}
					else if (isModeTeamBased())
					{
						m_observerPlayers[index].first.team = calculateNewPlayerTeam();
					}
					else 
					{
						m_observerPlayers[index].first.team = Common::Enums::TEAM_ALL;
					}
					auto playerInfo = std::move(m_observerPlayers[index]);
					std::uint32_t newTeam = playerInfo.first.team;
					m_players.push_back(std::move(playerInfo));
					m_observerPlayers.erase(m_observerPlayers.begin() + index);
					m_obsPlayerSessionIdToVecIdx.erase(uniqueId.session); 
					m_playerSessionIdToVecIdx[uniqueId.session] = m_players.size() - 1;
					return newTeam;
				}
				return std::nullopt; // Observer player cannot change to another team other than the first if...
			}
			return std::nullopt; 
		}

		bool Room::isModeTeamBased() const
		{
			return m_settings.mode == Common::Enums::TeamDeathMatch || m_settings.mode == Common::Enums::Elimination
				|| m_settings.mode == Common::Enums::CaptureTheBattery || m_settings.mode == Common::Enums::ItemMatch
				|| m_settings.mode == Common::Enums::BombBattle || m_settings.mode == Common::Enums::SniperMode
				|| m_settings.mode == Common::Enums::CloseCombat || m_settings.mode == Common::Enums::Scrimmage;
		}

		// Called when non-team-based mode changed to team-based-mode, and thus all teams go from '0' to either '1' or '2'.
		void Room::updatePlayersTeamToTeamBased()
		{
			for (std::size_t i = 0; auto& [roomPlayerInfo, session] : m_players)
			{
				roomPlayerInfo.team = (i % 2 == 0) ? Common::Enums::TEAM_BLUE : Common::Enums::TEAM_RED;
				++i;
			}
		}

		void Room::updatePlayersTeamToNonTeamBased()
		{
			for (auto& [roomPlayerInfo, session] : m_players)
			{
				roomPlayerInfo.team = Common::Enums::TEAM_ALL;
			}
		}
		
		Common::Enums::Team Room::calculateNewPlayerTeam() const
		{
			std::uint32_t totalBlueTeam = 0;
			std::uint32_t totalRedTeam = 0;

			for (const auto& [playerInfo, session] : m_players)
			{
				if (playerInfo.team == Common::Enums::Team::TEAM_BLUE) ++totalBlueTeam;
				else if (playerInfo.team == Common::Enums::Team::TEAM_RED) ++totalRedTeam;
			}
			if (totalRedTeam >= totalBlueTeam) return Common::Enums::TEAM_BLUE;
			return Common::Enums::TEAM_RED;
		}

		void Room::setPlayersPerTeam(std::uint16_t playersPerTeam)
		{
			m_settings.playersPerTeam = playersPerTeam;
		}

		std::uint8_t Room::getSpecificSetting() const
		{
			return m_specificSetting;
		}

		const std::string& Room::getRoomTitle() const
		{
			return m_title;
		}

		bool Room::isRoomFullObserverExcluded() const
		{
			// The check playersPerTeam == 0 is done because on square mode, with only 1 player as total players, it results in 1.
			const std::uint32_t maxPossiblePlayers = m_settings.playersPerTeam == 0 ? 1 : m_settings.playersPerTeam * 2;
			return static_cast<std::uint32_t>(m_players.size()) >= maxPossiblePlayers;
		}

		void Room::setStateFor(const Main::Structures::UniqueId& uniqueId, const Common::Enums::PlayerState& playerState) 
		{
			auto playerIt = m_playerSessionIdToVecIdx.find(uniqueId.session);
			if (playerIt != m_playerSessionIdToVecIdx.end())
			{
				auto index = playerIt->second;
				m_players[index].first.state = playerState; 
				m_players[index].second->setPlayerState(playerState); 
			}

			auto observerIt = m_obsPlayerSessionIdToVecIdx.find(uniqueId.session);
			if (observerIt != m_obsPlayerSessionIdToVecIdx.end())
			{
				auto index = observerIt->second; 
				m_observerPlayers[index].first.state = playerState; 
				m_observerPlayers[index].second->setPlayerState(playerState); 
			}
		}

		void Room::startMatch(const Main::Structures::UniqueId& uniqueId)
		{
			if (m_hasMatchStarted)
			{
				for (auto& [roomInfo, session] : ranges::views::concat(m_players, m_observerPlayers))
				{
					if (roomInfo.uniqueId == uniqueId)
					{
						Main::Details::sendPlayerState(*session, uniqueId);
						setStateFor(uniqueId, static_cast<Common::Enums::PlayerState>(2));
						session->setIsInMatch(true);
						return;
					}
				}
			}
			else
			{
				m_players[0].first.state = Common::Enums::STATE_PLAYING;
				m_players[0].second->setIsInMatch(true);
				for (auto& [roomInfo, session] : ranges::views::concat(m_players, m_observerPlayers))
				{
					if (roomInfo.state == Common::Enums::STATE_READY)
					{
						Main::Details::sendPlayerState(*session, uniqueId);
						setStateFor(uniqueId, static_cast<Common::Enums::PlayerState>(2));
						session->setIsInMatch(true);
					}
				}
				m_hasMatchStarted = true;
			}
		}

		bool Room::isObserverFull() const
		{
			return m_observerPlayers.size() >= 10;
		}

		bool Room::kickPlayer(const std::string& name)
		{
			for (auto& [roomInfo, session] : ranges::views::concat(m_players, m_observerPlayers))
			{
				if (std::string(session->getAccountInfo().nickname) == name)
				{
					// Order is important here, kick must happen before "removePlayer" call as it swaps sessions if the host is the one being kicked!
					if (session->getAccountInfo().playerGrade >= Common::Enums::PlayerGrade::GRADE_MOD) return false;
					m_kickedPlayerAccountIds.push_back(session->getAccountInfo().accountID);
					removePlayer(session, 0x23);
					return true;
				}
			}
			return false;
		}

		bool Room::wasPreviouslyKicked(std::uint32_t accountId) const
		{
			for (const auto& currentAccountId : m_kickedPlayerAccountIds)
			{
				if (accountId == currentAccountId) return true;
			}
			return false;
		}

		void Room::updateMap(std::uint16_t newMap)
		{
			m_settings.map = newMap;
		}

		void Room::updateRoomSettings(const Main::Structures::RoomSettingsUpdateBase& newRoomSettings, std::uint16_t newMode)
		{
			m_settings.isItemOn = newRoomSettings.isItemOn;
			m_settings.isOpen = newRoomSettings.isOpen;
			m_settings.map = newRoomSettings.map;
			m_settings.playersPerTeam = newRoomSettings.maxPlayers / 2;
			m_settings.time = newRoomSettings.time;
			m_settings.weaponRestriction = newRoomSettings.weaponRestriction;
			m_specificSetting = newRoomSettings.specificSetting;
			m_isTeamBalanceOn = false; // Team balance currently disabled as it's not implemented & causes team-bugs
			m_settings.mode = newMode;
		}

		Main::Structures::RoomSettingsUpdateTitlePassword Room::getRoomSettingsUpdate() const
		{
			Main::Structures::RoomSettingsUpdateTitlePassword ret;
			ret.roomSettingsUpdateBase.isItemOn = m_settings.isItemOn;
			ret.roomSettingsUpdateBase.isOpen = m_settings.isOpen;
			ret.roomSettingsUpdateBase.isTeamBalanceOn = false; // for now...
			ret.roomSettingsUpdateBase.map = m_settings.map;
			ret.roomSettingsUpdateBase.maxPlayers = m_settings.playersPerTeam == 0 ? 1 : m_settings.playersPerTeam * 2;
			std::memcpy(ret.password, m_password.c_str(), m_password.size());
			ret.roomSettingsUpdateBase.specificSetting = m_specificSetting;
			ret.roomSettingsUpdateBase.time = m_settings.time;
			std::memcpy(ret.title, m_title.c_str(), m_title.size());
			ret.roomSettingsUpdateBase.weaponRestriction = m_settings.weaponRestriction;

			return ret;
		}

		void Room::updateTitle(const std::string& newTitle)
		{
			m_title = newTitle;
		}

		void Room::updatePassword(const std::string& newPassword)
		{
			if (newPassword.empty())
			{
				m_settings.hasPassword = false;
			}
			else
			{
				m_settings.hasPassword = true;
			}
			m_password = newPassword;
		}

		void Room::addPoint(std::uint32_t team)
		{
			if (team == Common::Enums::TEAM_RED) ++m_redPoints;
			else ++m_bluePoints;
		}	

		std::uint32_t Room::getHostLevel() const
		{
			return m_players[0].second->getAccountInfo().playerLevel;
		}

		void Room::sendTo(const Main::Structures::UniqueId& uniqueId, const Common::Network::Packet& packet)
		{
			auto playerIter = m_playerSessionIdToVecIdx.find(uniqueId.session);
			if (playerIter != m_playerSessionIdToVecIdx.end())
			{
				m_players[playerIter->second].second->asyncWrite(packet);
				return;
			}

			auto observerIter = m_obsPlayerSessionIdToVecIdx.find(uniqueId.session);
			if (observerIter != m_obsPlayerSessionIdToVecIdx.end())
			{
				m_observerPlayers[observerIter->second].second->asyncWrite(packet);
			}
		}

		void Room::storeEndMatchStatsFor(const Main::Structures::UniqueId& uniqueId, const Main::Structures::ScoreboardResponse& stats, 
			std::uint32_t blueScore, std::uint32_t redScore, bool hasLeveledUp)
		{
			for (auto& [roomInfo, session] : m_players)
			{
				if (roomInfo.uniqueId == uniqueId)
				{
					Main::Enums::MatchEnd matchEnd;
					if (redScore == blueScore)
					{
						matchEnd = Main::Enums::MATCH_DRAW;
					}
					else if ((blueScore > redScore && roomInfo.team == Common::Enums::TEAM_BLUE)
						|| (redScore > blueScore && roomInfo.team == Common::Enums::TEAM_RED))
					{
						matchEnd = Main::Enums::MATCH_WON;
					}
					else
					{
						matchEnd = Main::Enums::MATCH_LOST;
					}
					session->storeEndMatchStats(stats, matchEnd, hasLeveledUp);
					return;
				}
			}
		}

		Main::Network::Session::AccountInfo Room::getAccountInfoFor(const Main::Structures::UniqueId& uniqueId) const
		{
			auto playerIter = m_playerSessionIdToVecIdx.find(uniqueId.session);
			if (playerIter != m_playerSessionIdToVecIdx.end())
			{
				return m_players[playerIter->second].second->getAccountInfo();
			}

			auto observerIter = m_obsPlayerSessionIdToVecIdx.find(uniqueId.session);
			if (observerIter != m_obsPlayerSessionIdToVecIdx.end())
			{
				return m_observerPlayers[observerIter->second].second->getAccountInfo();
			}
		}
	}
}
