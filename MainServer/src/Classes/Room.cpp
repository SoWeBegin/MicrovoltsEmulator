
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
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_upgradable_mutex.hpp>
#include "../../include/Handlers/Room/RoomLeaveHandler.h"
#include <Utils/IPC_Structs.h>
#include <Enums/PlayerEnums.h>
#include <Utils/Logger.h>

namespace Main
{
	namespace Classes
	{
		Room::Room(const std::string& title, const Main::Structures::RoomSettings& settings, const Main::Structures::RoomPlayerInfo& player, Main::Network::Session* session)
			: m_title{ title }, m_settings{ settings }, m_number{ ++idCounter }
		{
			const auto pair = std::pair{ player, session };
			m_players.push_back(pair);
			m_settings = settings;

			m_isTeamBalanceOn = false; // For now, team balance isn't supported as it causes issues e.g. team bug.
			//m_isTeamBalanceOn = isModeTeamBased() ? true : false; // Team balance is by default on for team-based modes
		}

		void Room::setTick(std::uint32_t tick)
		{
			m_tick = tick;
		}

		std::uint32_t Room::getPlayerIdx(std::uint64_t playerId) const
		{
			std::uint32_t idx = 0;
			for (const auto& player : m_players)
			{
				if (player.second->getId() == playerId)
				{
					return idx;
				}
				++idx;
			}
		}

		std::uint32_t Room::getTick() const
		{
			return m_tick;
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

		void Room::addPlayer(Main::Network::Session* session, std::uint32_t team)
		{
			const auto& accountInfo = session->getAccountInfo();
			Main::Structures::RoomPlayerInfo roomPlayerInfo;
			roomPlayerInfo.character = accountInfo.latestSelectedCharacter;
			roomPlayerInfo.level = accountInfo.playerLevel;
			roomPlayerInfo.ping = session->getPing();
			std::memcpy(roomPlayerInfo.playerName, accountInfo.nickname, 16);
			roomPlayerInfo.state = Common::Enums::STATE_WAITING;
			roomPlayerInfo.uniqueId = accountInfo.uniqueId;
			roomPlayerInfo.team = team;
			
			m_players.push_back(std::pair{ roomPlayerInfo, session });
			session->setRoomNumber(m_number);
			session->setIsInLobby(false);
		}

		void Room::updatePlayerInfo(Main::Network::Session* session)
		{
			for (auto& currentPlayer : m_players)
			{
				if (currentPlayer.second->getId() == session->getId())
				{
					const auto& accountInfo = session->getAccountInfo();
					currentPlayer.first.character = accountInfo.latestSelectedCharacter;
					currentPlayer.first.level = accountInfo.playerLevel;
					return;
				}
			}
			for (auto& currentPlayer : m_observerPlayers)
			{
				if (currentPlayer.second->getId() == session->getId())
				{
					const auto& accountInfo = session->getAccountInfo();
					currentPlayer.first.character = accountInfo.latestSelectedCharacter;
					currentPlayer.first.level = accountInfo.playerLevel;
					return;
				}
			}
		}

		std::size_t Room::getPlayersCount() const
		{
			return m_players.size();
		}

		void Room::addObserverPlayer(Main::Network::Session* session)
		{
			const auto& accountInfo = session->getAccountInfo();
			Main::Structures::RoomPlayerInfo roomPlayerInfo;
			roomPlayerInfo.ping = session->getPing();
			roomPlayerInfo.character = accountInfo.latestSelectedCharacter;
			roomPlayerInfo.level = accountInfo.playerLevel;
			std::memcpy(roomPlayerInfo.playerName, accountInfo.nickname, 16);
			roomPlayerInfo.uniqueId = accountInfo.uniqueId;
			roomPlayerInfo.team = Common::Enums::Team::TEAM_OBSERVER;
			m_observerPlayers.push_back(std::pair{ roomPlayerInfo, session });
			session->setRoomNumber(m_number);
			session->setIsInLobby(false);
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
			removePlayerFromRoomServerRequest.setOrder(141);
			removePlayerFromRoomServerRequest.setExtra(1);

			for (auto& player : m_players)
			{
				removePlayerFromRoomServerRequest.setTcpHeader(player.second->getId(), Common::Enums::USER_LARGE_ENCRYPTION);
				player.second->asyncWrite(removePlayerFromRoomServerRequest);
				player.second->leaveRoom();
			}
			for (auto& player : m_observerPlayers)
			{
				removePlayerFromRoomServerRequest.setTcpHeader(player.second->getId(), Common::Enums::USER_LARGE_ENCRYPTION);
				player.second->asyncWrite(removePlayerFromRoomServerRequest);
				player.second->leaveRoom();
			}
			m_players.resize(0);
			m_observerPlayers.resize(0);
		}

		// Returns the index of the player with the best ms, and if no player is found that meets the requirements, -1 is returned to signal that.
		std::uint32_t Room::getBestMsIndexExceptSelf(bool checkIsInMatch, std::uint64_t selfId)
		{
			std::uint32_t bestMs = -1;
			std::uint32_t bestMsPlayerIdx = 0;
			bool foundBestMs = false;
			std::uint32_t currentIdx = 0;
			for (const auto& currentPlayer : m_players)
			{
				std::cout << "getBestMsIndexExceptSelf.\n";
				std::cout << currentPlayer.second->getPlayerInfoAsString();
				std::cout << "SelfID = " << selfId << ", currentPlayerID: " << currentPlayer.second->getId() << '\n';
				std::cout << "Player Ping: " << currentPlayer.second->getPing() << '\n';
				std::cout << "bestMs so far: " << bestMs << '\n';
				if (selfId != currentPlayer.second->getId() && currentPlayer.second->getPing() < bestMs)
				{
					if (checkIsInMatch && currentPlayer.second->isInMatch())
					{
						bestMs = currentPlayer.second->getPing();
						bestMsPlayerIdx = currentIdx;
						foundBestMs = true;
					}
					else if (!checkIsInMatch)
					{
						bestMs = currentPlayer.second->getPing();
						bestMsPlayerIdx = currentIdx;
						foundBestMs = true;
					}
				}
				++currentIdx;
			}
			return foundBestMs ? bestMsPlayerIdx : -1;
		}

		// Returns true if the room has to also be closed, false otherwise
		// Checked
		bool Room::removeHostFromMatch()
		{
			Utils::Logger& logger = Utils::Logger::getInstance();

			auto originalHostUniqueId = m_players[0].second->getAccountInfo().uniqueId;
			const std::uint64_t originalHostSessionId = m_players[0].second->getId();

			Common::Network::Packet removePlayerFromRoomServerRequest;
			removePlayerFromRoomServerRequest.setTcpHeader(originalHostSessionId, Common::Enums::USER_LARGE_ENCRYPTION);
			removePlayerFromRoomServerRequest.setOrder(141);
			removePlayerFromRoomServerRequest.setExtra(1);

			Common::Network::Packet removePlayerFromMatchServerRequest;
			removePlayerFromMatchServerRequest.setTcpHeader(originalHostSessionId, Common::Enums::USER_LARGE_ENCRYPTION);
			removePlayerFromMatchServerRequest.setOrder(256);
			removePlayerFromMatchServerRequest.setExtra(0);
			removePlayerFromMatchServerRequest.setData(reinterpret_cast<std::uint8_t*>(&originalHostUniqueId), sizeof(originalHostUniqueId));

			Common::Network::Packet hostChange;
			hostChange.setTcpHeader(originalHostSessionId, Common::Enums::USER_LARGE_ENCRYPTION);
			hostChange.setOrder(128);


			// Signal that the host has been removed from the match
			m_players[0].second->setIsInMatch(false);

			// Count total players inside the match and total players inside the room
			std::uint32_t totalPlayersInMatch = 0;
			std::uint32_t totalPlayersInRoom = m_players.size();
			for (const auto& currentPlayer : m_players)
			{
				if (currentPlayer.second->isInMatch())
				{
					++totalPlayersInMatch;
				}
			}

			// Check if there are players inside the match that can be changed to the (match) host. 
			// We use >= 1 because the host has been removed from the match server side: there might be only one single player in the match, e.g. if this is square mode!
			if (totalPlayersInMatch >= 1)
			{
				logger.log("There are still some players in the match left, after the host left the match. Attempting to pick up new host...",
					Utils::LogType::Normal, "Room::removeHostFromMatch");

				bool hostChanged = false;

				// Find the player with best ms inside the match
				std::uint32_t bestMsPlayerIdxInMatch = getBestMsIndexExceptSelf(true, originalHostSessionId);

				// Switch the host to the player with the best ms inside the match.
				hostChanged = changeHost(bestMsPlayerIdxInMatch);
				if (hostChanged)
				{
					logger.log("The host has been changed from player " + std::string(m_players[bestMsPlayerIdxInMatch].second->getPlayerName())
						+ " to player: " + std::string(m_players[0].second->getPlayerName())
						+ ", idx of new host: " + std::to_string(bestMsPlayerIdxInMatch),
						Utils::LogType::Normal, "Room::removeHostFromMatch");

					hostChange.setOption(bestMsPlayerIdxInMatch);
					hostChange.setExtra(Common::Enums::CHANGE_HOST_SUCCESS);
					broadcastToRoom(hostChange);

					// Leave match + room packets.
					m_players[bestMsPlayerIdxInMatch].second->asyncWrite(removePlayerFromRoomServerRequest);
					broadcastToRoomExceptSelf(removePlayerFromMatchServerRequest, originalHostUniqueId);
					m_players[bestMsPlayerIdxInMatch].second->leaveRoom();
					Main::Handlers::notifyRoomPlayerLeaves(originalHostUniqueId, *this);

					logger.log("Removed previous host from room. " + m_players[bestMsPlayerIdxInMatch].second->getPlayerInfoAsString(),
						Utils::LogType::Normal, "Room::removeHostFromMatch");

					m_players.erase(m_players.begin() + bestMsPlayerIdxInMatch);
					// NOTE: This decrements all player indexes inside the client! Call it ONLY AFTER sending the packet to change the host!

					return false;
				}
				else
				{
					// Something went wrong: There were still some players in the match, but none of them could be picked up as the new host.
					// This can be caused b the "IsInMatch" flag being set incorrectly for some players. The server may think no players are in the match
					// even if they are.
					// In this case we cannot safely proceed further: Just notify the players about the error and close the room.
					hostChange.setExtra(Common::Enums::CHANGE_HOST_FAIL);
					broadcastToRoom(hostChange); // "Unknown error. Please restart the client" 

					logger.log("The host could not be switched to a new one! Closing the room to avoid further issues.",
						Utils::LogType::Error, "Room::removeHostFromMatch");

					return true;
				}

				logger.log(getRoomInfoAsString(), Utils::LogType::Normal, "Room::removeHostFromMatch");
			}

			// Otherwise, check if there are players inside the room that can be changed to the actual room host.
			// We use >1 and not >= 1 because the host is still counted in the room. 
			else if (totalPlayersInRoom > 1)
			{
				logger.log("The host is leaving and there are no players in the match. However, there are players in the room. Attempting to pick up new host...",
					Utils::LogType::Normal, "Room::removeHostFromMatch");

				std::uint32_t bestMsIndexPlayerOutsideMatch = getBestMsIndexExceptSelf(false, originalHostSessionId);

				bool hostChanged = changeHost(bestMsIndexPlayerOutsideMatch);
				if (hostChanged)
				{
					logger.log("The host has been changed from player " + std::string(m_players[bestMsIndexPlayerOutsideMatch].second->getPlayerName())
						+ " to player: " + std::string(m_players[0].second->getPlayerName())
						+ ", idx of new host: " + std::to_string(bestMsIndexPlayerOutsideMatch),
						Utils::LogType::Normal, "Room::removeHostFromMatch");

					// If a new match-host has been taken, notify whole room about it.
					hostChange.setOption(bestMsIndexPlayerOutsideMatch);
					hostChange.setExtra(Common::Enums::CHANGE_HOST_SUCCESS);
					broadcastToRoom(hostChange);

					// The previous host leaves the match, host->setIsInMatch(false) already set at this point
					broadcastToRoomExceptSelf(removePlayerFromMatchServerRequest, originalHostUniqueId);
					m_players[bestMsIndexPlayerOutsideMatch].second->asyncWrite(removePlayerFromRoomServerRequest);
					m_players[bestMsIndexPlayerOutsideMatch].second->leaveRoom();
					Main::Handlers::notifyRoomPlayerLeaves(originalHostUniqueId, *this);

					logger.log("Removed previous host from room. " + m_players[bestMsIndexPlayerOutsideMatch].second->getPlayerInfoAsString(),
						Utils::LogType::Normal, "Room::removeHostFromMatch");

					m_players.erase(m_players.begin() + bestMsIndexPlayerOutsideMatch);
					// NOTE: This decrements all player indexes inside the client! Call it ONLY AFTER sending the packet to change the host!
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

				logger.log(getRoomInfoAsString(), Utils::LogType::Normal, "Room::removeHostFromMatch");
				m_hasMatchStarted = false;
				return false; // Since in a correct implementation there was a player in the match that got the host, we shall not close the room.
			}

			// If the host was alone in the whole room (e.g. SquareMode, AI, BossBattle, and none outside the match), remove it from the room and close it.
			else
			{
				logger.log("The host is leaving and no other host can be picked up. Leaving the room and closing it...",
					Utils::LogType::Normal, "Room::removeHostFromMatch");

				m_players[0].second->asyncWrite(removePlayerFromRoomServerRequest);
				m_players[0].second->leaveRoom();
				m_players.resize(0);

				for (auto& obsPlayer : m_observerPlayers)
				{
					obsPlayer.second->asyncWrite(removePlayerFromRoomServerRequest);
					obsPlayer.second->leaveRoom();
				}
				m_observerPlayers.resize(0);
				m_hasMatchStarted = false;

				logger.log(getRoomInfoAsString(), Utils::LogType::Normal, "Room::removeHostFromMatch");
				return true; // The room should now be closed by the caller
			}
		}

		// Returns true if the room must be also be closed (e.g. due to host-switch errors, or because no other player is inside the room), false otherwise.
		// Checked
		bool Room::removePlayer(Main::Network::Session* session, std::uint32_t extra)
		{
			Utils::Logger& logger = Utils::Logger::getInstance();

			Common::Network::Packet removePlayerFromRoomServerRequest;
			removePlayerFromRoomServerRequest.setTcpHeader(session->getId(), Common::Enums::USER_LARGE_ENCRYPTION);
			removePlayerFromRoomServerRequest.setOrder(141);
			removePlayerFromRoomServerRequest.setExtra(extra);

			// For safety: check whether the given session exists inside "m_players"
			auto playerIter = std::find_if(m_players.begin(), m_players.end(),
				[session](const auto& currentSession) {
					return currentSession.second == session;
				});

			// If it does, we're taking care of a normal player (not an observer one)
			if (playerIter != m_players.end())
			{
				const std::uint32_t bestMsIndexPlayerOutsideMatch = getBestMsIndexExceptSelf(false, session->getId());

				// The player to be removed is the host. Find a new host.
				if (playerIter->second->getId() == m_players[0].second->getId())
				{
					auto uniqueId = playerIter->second->getAccountInfo().uniqueId;
					logger.log("The player " + session->getPlayerInfoAsString() + " is host and is attempting to leave their room", Utils::LogType::Normal, "Room::removePlayer");

					// if host is alone in room, getBestMsIndexExceptSelf returns -1, and changeHost will return false.
					bool hostChanged = changeHost(bestMsIndexPlayerOutsideMatch);
					if (hostChanged)
					{
						logger.log("The host has been changed from player " + std::string(m_players[bestMsIndexPlayerOutsideMatch].second->getPlayerName())
							+ " to player: " + std::string(m_players[0].second->getPlayerName())
							+ ", idx of new host: " + std::to_string(bestMsIndexPlayerOutsideMatch),
							Utils::LogType::Normal, "Room::removePlayer");

						// Notify room about the new host.
						Common::Network::Packet hostChange;
						hostChange.setTcpHeader(session->getId(), Common::Enums::USER_LARGE_ENCRYPTION);
						hostChange.setOrder(128);
						hostChange.setOption(bestMsIndexPlayerOutsideMatch);
						hostChange.setExtra(Common::Enums::CHANGE_HOST_SUCCESS);
						broadcastToRoom(hostChange);
					}
					else if (m_players.size() > 1) // Host not changed, even if it should have (because there were other players apart from the host inside the room)
					{
						// Notify whoever tried to switch host that it cannot be done
						Common::Network::Packet hostChange;
						hostChange.setTcpHeader(session->getId(), Common::Enums::USER_LARGE_ENCRYPTION);
						hostChange.setOrder(128);
						hostChange.setExtra(Common::Enums::CHANGE_HOST_FAIL);
						broadcastToRoom(hostChange); // "Unknown error. Please restart the client" 

						logger.log("The host could not be switched to a new one! Closing the room to avoid further issues.",
							Utils::LogType::Error, "Room::removePlayer");

						return true; // Something went wrong while trying to switch to a new host after the previous one left the room.
						// In this case, just close the room to avoid further issues.
					}
				}

				// At this point, playerIter is invalidated. Refind it.
				playerIter = std::find_if(m_players.begin(), m_players.end(),
					[session](const auto& currentSession) {
						return currentSession.second == session;
					});

				// If we're here, that means that the host was successfully changed and we can now remove the previous host from the room.
				// NOTE: This decrements all player indexes in the client!!! Do it always AFTER sending the change host packet!
				playerIter->second->asyncWrite(removePlayerFromRoomServerRequest);
				playerIter->second->leaveRoom();
				Main::Handlers::notifyRoomPlayerLeaves(playerIter->second->getAccountInfo().uniqueId, *this);
				m_players.erase(playerIter);

				logger.log("Removed player from room. " + playerIter->second->getPlayerInfoAsString(),
					Utils::LogType::Normal, "Room::removePlayer");
				logger.log(getRoomInfoAsString(), Utils::LogType::Normal, "Room::removePlayer");

				return m_players.empty(); // If empty, the room must be also closed.
			}

			// Otherwise, we're taking care of a player that is in observer mode. Check if that's the case and whether it actually exists in m_observerPlayers.
			auto observerIter = std::find_if(m_observerPlayers.begin(), m_observerPlayers.end(),
				[session](const auto& currentSession) {
					return currentSession.second == session;
				});
			if (observerIter != m_observerPlayers.end())
			{
				logger.log("Removed observer player from room. " + observerIter->second->getPlayerInfoAsString(),
					Utils::LogType::Normal, "Room::removePlayer");

				Main::Handlers::notifyRoomPlayerLeaves(observerIter->second->getAccountInfo().uniqueId, *this);
				observerIter->second->asyncWrite(removePlayerFromRoomServerRequest);
				observerIter->second->leaveRoom();
				m_observerPlayers.erase(observerIter);
				// NOTE: This decrements all player indexes in the client!!! Do it always AFTER sending the change host packet!
			}

			logger.log(getRoomInfoAsString(), Utils::LogType::Normal, "Room::removePlayer");
			return m_players.empty();
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

		void Room::removeHostIfAloneAndModeDoesntAllowIt()
		{
			std::uint32_t totalInMatch = 0;
			for (const auto& currentPlayer : m_players)
			{
				if (currentPlayer.second->isInMatch())
				{
					++totalInMatch;
				}
			}

			if (totalInMatch == 1)
			{
				if (m_settings.mode == Common::Enums::AiBattle || m_settings.mode == Common::Enums::SquareMode || m_settings.mode == Common::Enums::BossBattle)
				{
					return; // These modes allow the player to play alone in the match.
				}
				std::cout << "Only host left in match. Leaving it...\n";
				m_players[0].second->setIsInMatch(false);
				setStateFor(m_players[0].second->getAccountInfo().uniqueId, Common::Enums::STATE_NORMAL);

				// Also update for observer players!
				for (auto& observerPlayer : m_observerPlayers)
				{
					if (observerPlayer.second->isInMatch())
					{
						observerPlayer.second->setIsInMatch(false);
						setStateFor(observerPlayer.second->getAccountInfo().uniqueId, Common::Enums::STATE_NORMAL);
					}
				}
			}
		}


		Main::Structures::SingleRoom Room::getRoomInfo() const
		{
			Main::Structures::SingleRoom roomInfo;
			roomInfo.hasPassword = m_password.empty() ? 0 : 1;
			roomInfo.isObserverOff = m_settings.isObserverModeOn ? 0 : 1;
			roomInfo.map = m_settings.map;
			roomInfo.matchStarted = m_hasMatchStarted;
			roomInfo.maxPlayers = m_settings.playersPerTeam == 0 ? 1 : m_settings.playersPerTeam * 2;
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

		// Specific setting is missing here!!!!!
		Main::Structures::RoomJoin Room::getRoomJoinInfo() const
		{
			Main::Structures::RoomJoin roomJoinInfo;
			roomJoinInfo.hasPassword = m_password.empty() ? 0 : 1;
			roomJoinInfo.isObserverOn = m_settings.isObserverModeOn;
			roomJoinInfo.isOpen = m_settings.isOpen;
			roomJoinInfo.map = m_settings.map;
			roomJoinInfo.mode = m_settings.mode;
			roomJoinInfo.hidePassword = false; //m_password.empty() ? 0 : 1;
			roomJoinInfo.maxPlayers = m_settings.playersPerTeam == 0 ? 1 : m_settings.playersPerTeam * 2;
			roomJoinInfo.weaponRestriction = m_settings.weaponRestriction;
			roomJoinInfo.isTeamBalanceOn = m_isTeamBalanceOn;
			roomJoinInfo.hasMatchStarted = m_hasMatchStarted;
			std::memcpy(roomJoinInfo.password, m_password.data(), 9); // password max according to client = 8 characters
			return roomJoinInfo;
		}

		std::vector<Main::Structures::RoomPlayerItems> Room::getPlayersItems() const
		{
			std::vector<Main::Structures::RoomPlayerItems> ret;
			for (const auto& currentPlayer : m_players)
			{
				Main::Structures::RoomPlayerItems roomPlayerItems;
				auto separatedItems = currentPlayer.second->getEquippedItemsSeparated(); // first=items, second=weapons
				roomPlayerItems.equippedItems = separatedItems.first;
				roomPlayerItems.equippedWeapons = separatedItems.second;
				roomPlayerItems.uniqueId = currentPlayer.first.uniqueId;
				ret.push_back(roomPlayerItems);
			}
			for (const auto& currentPlayer : m_observerPlayers)
			{
				Main::Structures::RoomPlayerItems roomPlayerItems;
				auto separatedItems = currentPlayer.second->getEquippedItemsSeparated(); // first=items, second=weapons
				roomPlayerItems.equippedItems = separatedItems.first;
				roomPlayerItems.equippedWeapons = separatedItems.second;
				roomPlayerItems.uniqueId = currentPlayer.first.uniqueId;
				ret.push_back(roomPlayerItems);
			}
			return ret;
		}

		std::vector<Main::Structures::PlayerClan> Room::getPlayersClans() const
		{
			std::vector<Main::Structures::PlayerClan> ret;
			for (const auto& currentPlayer : m_players)
			{
				const auto& accountInfo = currentPlayer.second->getAccountInfo();
				Main::Structures::PlayerClan playerClan;
				const bool hasClan = accountInfo.clanId >= 8;
				if (hasClan) // player has clan
				{
					playerClan.clanLogoBackId = accountInfo.clanLogoBackId;
					playerClan.clanLogoFrontId = accountInfo.clanLogoFrontId;
					std::memcpy(playerClan.clanName, accountInfo.clanName, 16);
					playerClan.unknown2 = accountInfo.clanId; // unsure whether this is correct!
					ret.push_back(playerClan);
				}
				else
				{
					playerClan.unknown = 1;
					ret.push_back(playerClan);
				}
			}
			for (const auto& currentPlayer : m_observerPlayers)
			{
				const auto& accountInfo = currentPlayer.second->getAccountInfo();
				Main::Structures::PlayerClan playerClan;
				const bool hasClan = accountInfo.clanId >= 8;
				if (hasClan) // player has clan
				{
					playerClan.clanLogoBackId = accountInfo.clanLogoBackId;
					playerClan.clanLogoFrontId = accountInfo.clanLogoFrontId;
					std::memcpy(playerClan.clanName, accountInfo.clanName, 16);
					playerClan.unknown2 = accountInfo.clanId; // unsure whether this is correct!
					ret.push_back(playerClan);
				}
				else
				{
					playerClan.unknown = 1;
					ret.push_back(playerClan);
				}
			}
			return ret;
		}

		void Room::breakroom()
		{
			for (const auto& currentPlayer : m_players)
			{
				currentPlayer.second->leaveRoom();
			}
			for (const auto& currentObserverPlayer : m_observerPlayers)
			{
				currentObserverPlayer.second->leaveRoom();
			}
			m_players.resize(0);
			m_observerPlayers.resize(0);
		}

		void Room::broadcastToRoom(Common::Network::Packet& packet, bool broadcastToMatch)
		{
			for (const auto& currentPlayer : m_players)
			{
				packet.setTcpHeader(currentPlayer.second->getSessionId(), Common::Enums::USER_LARGE_ENCRYPTION);
				currentPlayer.second->asyncWrite(packet);
			}
			for (const auto& currentObserverPlayer : m_observerPlayers)
			{
				packet.setTcpHeader(currentObserverPlayer.second->getSessionId(), Common::Enums::USER_LARGE_ENCRYPTION);
				currentObserverPlayer.second->asyncWrite(packet);
			}
		}

		void Room::broadcastToRoomExceptSelf(Common::Network::Packet& packet, const Main::Structures::UniqueId& uniqueId)
		{
			for (const auto& currentPlayer : m_players)
			{
				if (currentPlayer.first.uniqueId == uniqueId) continue;
				packet.setTcpHeader(currentPlayer.second->getSessionId(), Common::Enums::USER_LARGE_ENCRYPTION);
				currentPlayer.second->asyncWrite(packet);
			}
			for (const auto& currentObserverPlayer : m_observerPlayers)
			{
				if (currentObserverPlayer.first.uniqueId == uniqueId) continue;
				packet.setTcpHeader(currentObserverPlayer.second->getSessionId(), Common::Enums::USER_LARGE_ENCRYPTION);
				currentObserverPlayer.second->asyncWrite(packet);
			}
		}

		void Room::broadcastToMatchExceptSelf(Common::Network::Packet& packet, const Main::Structures::UniqueId& uniqueId, std::uint32_t extra)
		{
			if (extra == Main::Enums::ChatExtra::TEAM)
			{
				Common::Enums::Team targetTeam = Common::Enums::Team::TEAM_ALL;
				// Find target team
				for (const auto& currentPlayer : m_players)
				{
					if (currentPlayer.first.uniqueId == uniqueId)
					{
						targetTeam = static_cast<Common::Enums::Team>(currentPlayer.first.team);
						break;
					}
				}
				// Broadcast to target team
				for (const auto& currentPlayer : m_players)
				{
					if (currentPlayer.first.uniqueId == uniqueId || currentPlayer.first.team != targetTeam || !currentPlayer.second->isInMatch()) continue;
					packet.setTcpHeader(currentPlayer.second->getSessionId(), Common::Enums::USER_LARGE_ENCRYPTION);
					currentPlayer.second->asyncWrite(packet);
				}
			}

			else
			{
				bool broadcastOnlyToObs = false;
				for (const auto& currentObserverPlayer : m_observerPlayers)
				{
					if (currentObserverPlayer.first.uniqueId == uniqueId)
					{
						if (currentObserverPlayer.second->getAccountInfo().playerGrade < Common::Enums::GRADE_MOD) // >= MOD grade messages are broadcast to whole match anyway.
						{
							broadcastOnlyToObs = true;
						}
						continue;
					}
					if (currentObserverPlayer.second->isInMatch())
					{
						packet.setTcpHeader(currentObserverPlayer.second->getSessionId(), Common::Enums::USER_LARGE_ENCRYPTION);
						currentObserverPlayer.second->asyncWrite(packet);
					}
				}

				if (!broadcastOnlyToObs)
				{
					for (const auto& currentPlayer : m_players)
					{
						if (currentPlayer.first.uniqueId == uniqueId) continue;
						if (currentPlayer.second->isInMatch())
						{
							packet.setTcpHeader(currentPlayer.second->getSessionId(), Common::Enums::USER_LARGE_ENCRYPTION);
							currentPlayer.second->asyncWrite(packet);
						}
					}
				}
			}
		}

		void Room::broadcastOutsideMatchExceptSelf(Common::Network::Packet& packet, const Main::Structures::UniqueId& uniqueId, std::uint32_t extra)
		{
			if (extra == Main::Enums::ChatExtra::TEAM)
			{
				Common::Enums::Team targetTeam = Common::Enums::Team::TEAM_ALL;
				// Find target team
				for (const auto& currentPlayer : m_players)
				{
					if (currentPlayer.first.uniqueId == uniqueId)
					{
						targetTeam = static_cast<Common::Enums::Team>(currentPlayer.first.team);
						break;
					}
				}
				// Broadcast to target team
				for (const auto& currentPlayer : m_players)
				{
					if (currentPlayer.first.uniqueId == uniqueId || currentPlayer.second->isInMatch() || currentPlayer.first.team != targetTeam) continue;
					packet.setTcpHeader(currentPlayer.second->getSessionId(), Common::Enums::USER_LARGE_ENCRYPTION);
					currentPlayer.second->asyncWrite(packet);
				}
			}

			else
			{
				for (const auto& currentPlayer : m_players)
				{
					if (currentPlayer.first.uniqueId == uniqueId || currentPlayer.second->isInMatch()) continue;
					packet.setTcpHeader(currentPlayer.second->getSessionId(), Common::Enums::USER_LARGE_ENCRYPTION);
					currentPlayer.second->asyncWrite(packet);
				}
				for (const auto& currentObserverPlayer : m_observerPlayers)
				{
					if (currentObserverPlayer.first.uniqueId == uniqueId || currentObserverPlayer.second->isInMatch()) continue;
					packet.setTcpHeader(currentObserverPlayer.second->getSessionId(), Common::Enums::USER_LARGE_ENCRYPTION);
					currentObserverPlayer.second->asyncWrite(packet);
				}
			}
		}

		void Room::endMatch()
		{
			Common::Network::Packet response;
			response.setTcpHeader(m_players[0].second->getId(), Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(312);
			m_hasMatchStarted = false;

			for (auto& currentPlayer : m_players)
			{
				currentPlayer.first.state = Common::Enums::STATE_WAITING;
				currentPlayer.second->setIsInMatch(false);
			}
			for (auto& currentPlayer : m_observerPlayers)
			{
				if (currentPlayer.second->isInMatch()) 
				{
					currentPlayer.first.state = Common::Enums::STATE_WAITING;
					currentPlayer.second->setIsInMatch(false);
				}
			}
		}

		// Checked
		bool Room::changeHost(std::size_t newHostIdx)
		{
			Utils::Logger& logger = Utils::Logger::getInstance();

			auto previousHostUniqueId = m_players[0].second->getAccountInfo().uniqueId;

			if (newHostIdx >= m_players.size())
			{
				logger.log("The newHostIdx is not found inside m_players! (newHostIdx: " + std::to_string(newHostIdx) + ", m_players.size(): " + std::to_string(m_players.size()),
					Utils::LogType::Normal, "Room::changeHost");
				return false;
			}

			std::swap(m_players[0], m_players[newHostIdx]); 
			return true;
		}

		Main::Structures::UniqueId Room::getHostUniqueId() const
		{
			return m_players[0].first.uniqueId;
		}

		Main::Network::Session* Room::getPlayer(const Main::Structures::UniqueId& uniqueId)
		{
			for (const auto& [roomInfo, session] : m_players)
			{
				if (roomInfo.uniqueId == uniqueId)
				{
					return session;
				}
			}
			std::cout << "Player Not Found inside Room::getPlayer\n";
			return nullptr;
		}


		// At this point, the host has already left the room, that is: host->isInMatch() == false.

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
		std::optional<std::uint32_t> Room::changePlayerTeam(const Main::Structures::UniqueId& uniqueId, std::uint32_t newTeam)
		{
			Utils::Logger& logger = Utils::Logger::getInstance();

			for (auto it = m_players.begin(); it != m_players.end(); ++it)
			{
				if (it->first.uniqueId == uniqueId)
				{
					if (newTeam == Common::Enums::TEAM_OBSERVER && m_observerPlayers.size() < 10)
					{
						auto playerInfo = std::move(*it);
						playerInfo.first.team = Common::Enums::TEAM_OBSERVER;
						m_observerPlayers.push_back(std::move(playerInfo));
						m_players.erase(it);

						logger.log("The player " + playerInfo.second->getPlayerInfoAsString() + " changed team from Player to Observer",
							Utils::LogType::Normal, "Room::switchPlayerTeam");
					}
					else
					{
						// Do NOT update if team is not BLUE or RED. That's just for the client to know (e.g. Zombie team)
						//if (newTeam != Common::Enums::TEAM_BLUE && newTeam != Common::Enums::TEAM_RED && newTeam != Common::Enums::TEAM_ALL) return newTeam;
						//it->first.team = it->first.team == Common::Enums::TEAM_BLUE ? Common::Enums::TEAM_RED : Common::Enums::TEAM_BLUE;

						if (newTeam == Common::Enums::TEAM_ZOMBIE)
						{
							it->first.team = newTeam;
						}
						else
						{
							it->first.team = it->first.team == Common::Enums::TEAM_BLUE ? Common::Enums::TEAM_RED : Common::Enums::TEAM_BLUE;
						}

						logger.log("The player " + it->second->getPlayerInfoAsString() + " changed team to: " + std::to_string(it->first.team),
							Utils::LogType::Normal, "Room::switchPlayerTeam");
					}
					return newTeam;
				}
			}
			for (auto it = m_observerPlayers.begin(); it != m_observerPlayers.end(); ++it)
			{
				if (it->first.uniqueId == uniqueId)
				{
					if (newTeam == Common::Enums::TEAM_ALL || newTeam == Common::Enums::TEAM_RED || newTeam == Common::Enums::TEAM_BLUE)
					{
						if (m_players.size() >= m_settings.playersPerTeam * 2)
						{
							logger.log("Could not switch from ObserverTeam => PlayerTeam because there are too many players in the room",
								Utils::LogType::Warning, "Room::switchPlayerTeam");
							return std::nullopt;
						}

						if (isModeTeamBased())
						{
							it->first.team = calculateNewPlayerTeam();
						}
						else
						{
							it->first.team = Common::Enums::TEAM_ALL;
						}
						auto playerInfo = std::move(*it);
						std::uint32_t newTeam = playerInfo.first.team;
						m_players.push_back(std::move(playerInfo));
						m_observerPlayers.erase(it);

						logger.log("The Observer Player " + playerInfo.second->getPlayerInfoAsString() + " changed team from Observer to: " 
							+ std::to_string(playerInfo.first.team),
							Utils::LogType::Normal, "Room::switchPlayerTeam");

						return newTeam;
					}
					else
					{
						// Makes no sense: observer player cannot change to another team other than the first if... 
						return std::nullopt;

						logger.log("The Observer Player " + it->second->getPlayerInfoAsString() + " tried to change the team from Observer to UnknownTeam: "
							+ std::to_string(newTeam), Utils::LogType::Warning, "Room::switchPlayerTeam");
					}
					return std::nullopt;
				}
			}
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
			Utils::Logger& logger = Utils::Logger::getInstance();

			for (std::size_t i = 0; auto& [roomPlayerInfo, session] : m_players)
			{
				roomPlayerInfo.team = (i % 2 == 0) ? Common::Enums::TEAM_BLUE : Common::Enums::TEAM_RED;
				++i;

				logger.log(std::string(session->getAccountInfo().nickname) + " new team: " +
					(roomPlayerInfo.team == Common::Enums::TEAM_BLUE ? "TEAM_BLUE" : "TEAM_RED"),
					::Utils::LogType::Info, "Room::updatePlayersTeam");
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
			Utils::Logger& logger = Utils::Logger::getInstance();
			logger.log("Total BLUE team: " + std::to_string(totalBlueTeam) + ", Total RED team: " + std::to_string(totalRedTeam),
				Utils::LogType::Normal, "Room::calculateNewPlayerTeam");

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

		bool Room::isRoomFullObserverExcluded() const
		{
			// The check playersPerTeam == 0 is done because on square mode, with only 1 player as total players, apparently it results in 1.
			const std::uint32_t maxPossiblePlayers = m_settings.playersPerTeam == 0 ? 1 : m_settings.playersPerTeam * 2;
			return static_cast<std::uint32_t>(m_players.size()) >= maxPossiblePlayers;
		}

		void Room::setStateFor(const Main::Structures::UniqueId& uniqueId, const Common::Enums::PlayerState& playerState)
		{
			for (auto& currentPlayer : m_players)
			{
				if (currentPlayer.first.uniqueId == uniqueId)
				{
					currentPlayer.first.state = playerState;
					currentPlayer.second->setPlayerState(playerState);
					return;
				}
			}
			for (auto& currentPlayer : m_observerPlayers)
			{
				if (currentPlayer.first.uniqueId == uniqueId)
				{
					currentPlayer.first.state = playerState;
					currentPlayer.second->setPlayerState(playerState);
					return;
				}
			}
		}

		void Room::startMatch(const Main::Structures::UniqueId& uniqueId)
		{
			if (m_hasMatchStarted)
			{
				for (auto& currentPlayer : m_players)
				{
					if (currentPlayer.first.uniqueId == uniqueId)
					{
						currentPlayer.first.state = Common::Enums::STATE_NORMAL;
						currentPlayer.second->setIsInMatch(true);
						return;
					}
				}
				for (auto& currentPlayer : m_observerPlayers)
				{
					if (currentPlayer.first.uniqueId == uniqueId)
					{
						currentPlayer.first.state = Common::Enums::STATE_NORMAL;
						currentPlayer.second->setIsInMatch(true);
						return;
					}
				}
			}
			else
			{
				m_hasMatchStarted = true;
				for (auto& currentPlayer : m_players)
				{
					if (currentPlayer.first.state == Common::Enums::STATE_READY)
					{
						currentPlayer.first.state = Common::Enums::STATE_NORMAL;
						currentPlayer.second->setIsInMatch(true);
					}
				}
				for (auto& currentPlayer : m_observerPlayers)
				{
					if (currentPlayer.first.state == Common::Enums::STATE_READY)
					{
						currentPlayer.first.state = Common::Enums::STATE_NORMAL;
						currentPlayer.second->setIsInMatch(true);
					}
				}
			}
		}

		bool Room::isObserverFull() const
		{
			return m_observerPlayers.size() >= 10;
		}

		bool Room::kickPlayer(const std::string& name)
		{
			for (const auto& player : m_players)
			{
				if (std::string(player.second->getAccountInfo().nickname) == name)
				{
					if (player.second->getAccountInfo().playerGrade >= Common::Enums::PlayerGrade::GRADE_MOD) return false;
					Common::Network::Packet response;
					response.setTcpHeader(player.second->getId(), Common::Enums::USER_LARGE_ENCRYPTION);
					response.setOrder(141);
					response.setExtra(0x23);
					player.second->asyncWrite(response);
					removePlayer(player.second, 0x23);
					addKickedPlayer(player.second->getAccountInfo().accountID);
					player.second->setIsInLobby(true);
					return true;
				}
			}
			for (const auto& player : m_observerPlayers)
			{
				if (std::string(player.second->getAccountInfo().nickname) == name)
				{
					if (player.second->getAccountInfo().playerGrade >= Common::Enums::PlayerGrade::GRADE_MOD) return false;
					Common::Network::Packet response;
					response.setTcpHeader(player.second->getId(), Common::Enums::USER_LARGE_ENCRYPTION);
					response.setOrder(141);
					response.setExtra(0x23);
					player.second->asyncWrite(response);
					removePlayer(player.second, 0x23);
					addKickedPlayer(player.second->getAccountInfo().accountID);
					player.second->setIsInLobby(true);
					return true;
				}
			}
			return false;
		}

		bool Room::wasPreviouslyKicked(std::uint32_t accountId) const
		{
			for (const auto& currentAccountId : m_kickedPlayerAccountIds)
			{
				if (accountId == currentAccountId)
				{
					return true;
				}
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
			std::cout << "Update Password: " << newPassword << '\n';
			m_password = "";
			if (newPassword.empty())
			{
				std::cout << "HasPassword = false\n";
				m_settings.hasPassword = false;
			}
			else
			{
				std::cout << "HasPassword = true\n";
				m_password = newPassword;
				m_settings.hasPassword = true;
			}
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
			for (auto& currentPlayer : m_players)
			{
				if (currentPlayer.first.uniqueId == uniqueId)
				{
					currentPlayer.second->asyncWrite(packet);
					return;
				}
			}
			for (auto& currentPlayer : m_observerPlayers)
			{
				if (currentPlayer.first.uniqueId == uniqueId)
				{
					currentPlayer.second->asyncWrite(packet);
					return;
				}
			}
		}

		void Room::storeEndMatchStatsFor(const Main::Structures::UniqueId& uniqueId, const Main::Structures::ScoreboardResponse& stats,
			std::uint32_t blueScore, std::uint32_t redScore, bool hasLeveledUp)
		{
			for (auto& currentPlayer : m_players)
			{
				if (currentPlayer.first.uniqueId == uniqueId)
				{
					std::cout << "storeEndMatchStatsFor: UniqueID found\n";
					Main::Enums::MatchEnd matchEnd;
					if (redScore == blueScore)
					{
						matchEnd = Main::Enums::MATCH_DRAW;
					}
					else if ((blueScore > redScore && currentPlayer.first.team == Common::Enums::TEAM_BLUE)
						|| (redScore > blueScore && currentPlayer.first.team == Common::Enums::TEAM_RED))
					{
						matchEnd = Main::Enums::MATCH_WON;
					}
					else
					{
						matchEnd = Main::Enums::MATCH_LOST;
					}
					currentPlayer.second->storeEndMatchStats(stats, matchEnd, hasLeveledUp);
					return;
				}
			}
		}

		Main::Network::Session::AccountInfo Room::getAccountInfoFor(const Main::Structures::UniqueId& uniqueId) const
		{
			for (auto& currentPlayer : m_players)
			{
				if (currentPlayer.first.uniqueId == uniqueId)
				{
					return currentPlayer.second->getAccountInfo();
				}
			}
		}
	}
}
