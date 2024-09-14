
#include "Network/Session.h"
#include "../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "../../include/Structures/Item/MainItem.h"
#include "../../include/Structures/Item/MainEquippedItem.h"
#include "../../include/Persistence/MainScheduler.h"
#include "../../include/Persistence/MainDatabaseManager.h"
#include "../../include/Structures/PlayerLists/BlockedPlayer.h"
#include "../../include/Structures/PlayerLists/Friend.h"
#include "../../include/Structures/TradeSystem/TradeSystemItem.h"
#include "../../include/Structures/Mailbox.h"
#include "Enums/GameEnums.h"

#include "../../include/Classes/Player.h"
#include "../../include/Network/MainSession.h"
#include <functional>
#include <chrono>
#include <asio.hpp>

namespace Main
{
	namespace Network
	{
		Session::Session(Main::Persistence::MainScheduler& scheduler, tcp::socket&& socket, std::function<void(std::size_t)> fnct)
			: Common::Network::Session{ std::move(socket), fnct }
			, m_scheduler{ scheduler }
		{
		}

		std::size_t Session::getSessionId() const
		{
			return m_id;
		}

		void Session::onPacket(std::vector<std::uint8_t>& data)
		{
			std::optional<std::uint32_t> key = std::nullopt;
			if (m_crypt.isUsed) key = m_crypt.UserKey;

			Common::Network::Packet incomingPacket;
			incomingPacket.processIncomingPacket(data.data(), static_cast<std::uint16_t>(data.size()), key);

			const std::uint16_t callbackNum = incomingPacket.getOrder();
			if (!Common::Network::Session::callbacks<Session>.contains(callbackNum)) 
			{
				std::cout << "[MainSession] No callback for order: " << callbackNum << "\n";
				return;
			}

			Common::Network::Session::callbacks<Session>[callbackNum](incomingPacket, *this);
		}

		void Session::addMailboxReceived(const Main::Structures::Mailbox& mailbox)
		{
			m_player.addMailboxReceived(mailbox);
			m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::storeMailbox,
				mailbox, false);
		}

		void Session::addMailboxSent(const Main::Structures::Mailbox& mailbox)
		{
			m_player.addMailboxSent(mailbox);
			m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::storeMailbox,
				mailbox, true);
		}

		bool Session::deleteSentMailbox(std::uint32_t timestamp, std::uint32_t accountId)
		{
			if (m_player.deleteSentMailbox(timestamp, accountId))
			{
				m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::deleteMailbox,
					timestamp, accountId, true);
				return true;
			}
			return false;
		}

		bool Session::deleteReceivedMailbox(std::uint32_t timestamp, std::uint32_t accountId)
		{
			if (m_player.deleteReceivedMailbox(timestamp, accountId))
			{
				m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::deleteMailbox,
					timestamp, accountId, false);
				return true;
			}
			return false;
		}

		const std::vector<Main::Structures::Mailbox>& Session::getMailboxReceived() const
		{
			return m_player.getMailboxReceived();
		}

		const std::vector<Main::Structures::Mailbox>& Session::getMailboxSent() const
		{
			return m_player.getMailboxSent();
		}

		void Session::setMailbox(const std::vector<Main::Structures::Mailbox>& mailbox, bool sent)
		{
			m_player.setMailbox(mailbox, sent);
		}

		void Session::setAccountInfo(const AccountInfo& accountInfo)
		{
			m_player.setAccountInfo(accountInfo);
		}

		void Session::addBatteryObtainedInMatch(std::uint32_t newBattery)
		{
			m_player.addBatteryObtainedInMatch(newBattery);
		}

		const Main::Structures::AccountInfo& Session::getAccountInfo() const
		{
			return m_player.getAccountInfo();
		}

		const std::vector<Main::Structures::Friend> Session::getFriendlist() const
		{
			return m_player.getFriendlist();
		}

		std::unordered_map<Main::Structures::Friend, Session*>& Session::getFriendSessions()
		{
			return m_player.getFriendSessions();
		}

		const char* const Session::getPlayerName() const
		{
			return m_player.getPlayerName();
		}

		void Session::sendAccountInfo(Common::Network::Packet& response)
		{
			auto accountInfo = m_player.getAccountInfo();
			response.setData(reinterpret_cast<std::uint8_t*>(&accountInfo), sizeof(accountInfo));
			asyncWrite(response);
		}

		void Session::setPing(std::uint16_t ping)
		{
			m_player.setPing(ping);
		}

		std::uint16_t Session::getPing() const
		{
			return m_player.getPing();
		}

		void Session::setUnequippedItems(const std::vector<Item>& items)
		{
			m_player.setUnequippedItems(items);
		}

		void Session::setFriendList(const std::vector<Main::Structures::Friend>& friendlist)
		{
			m_player.setFriendList(friendlist);
		}

		void Session::logFriend(std::uint8_t logType, std::uint32_t targetAccountId)
		{
			Common::Network::Packet response;
			response.setTcpHeader(m_id, Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(85); //Friend login/logout
			response.setExtra(53); // Login logout
			response.setMission(0);
			response.setOption(logType); // 46 == login, anything else == logout
			response.setData(reinterpret_cast<std::uint8_t*>(&targetAccountId), sizeof(targetAccountId));
			asyncWrite(response);
		}

		void Session::updateFriendSession(Session* targetSession, bool remove)
		{
			if (targetSession)
			{
				Main::Structures::Friend targetFriend;
				const auto& targetAccountInfo = targetSession->getAccountInfo();
				targetFriend.targetAccountId = targetAccountInfo.accountID;
				targetFriend.targetUniqueId = remove ? Main::Structures::UniqueId{} : targetAccountInfo.uniqueId;
				std::memcpy(targetFriend.targetNickname, targetAccountInfo.nickname, 16);
				m_player.updateFriend(targetFriend, targetSession, remove);
			}
		}

		void Session::blockAccount(std::uint32_t accountId, const char* nickname)
		{
			m_player.blockAccount(accountId, nickname);
			m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::blockPlayer,
				m_player.getAccountID(), accountId);
		}

		bool Session::unblockAccount(std::uint32_t accountId)
		{
			if (m_player.unblockAccount(accountId))
			{
				m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::unblockPlayer,
					m_player.getAccountID(), accountId);
				return true;
			}
			return false;
		}

		bool Session::hasBlocked(std::uint32_t accountId) const
		{
			return m_player.hasBlocked(accountId);
		}

		const std::vector<Main::Structures::BlockedPlayer>& Session::getBlockedPlayers() const
		{
			return m_player.getBlockedPlayers();
		}

		void Session::setBlockedPlayers(const std::vector<Main::Structures::BlockedPlayer>& blockedPlayers)
		{
			m_player.setBlockedPlayers(blockedPlayers);
		}

		// call once with default "persist", since removeFriend removes the friend for both players
		void Session::deleteFriend(std::uint32_t targetAccountId, bool persist)
		{
			m_player.deleteFriend(targetAccountId);
			if (persist)
			{
				m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::removeFriend,
					m_player.getAccountID(), targetAccountId);
			}
		}

		const std::optional<Main::Structures::Item> Session::findItemBySerialInfo(const Main::Structures::ItemSerialInfo& itemSerialInfo) const
		{
			return m_player.findItemBySerialInfo(itemSerialInfo);
		}

		void Session::replaceItem(const std::uint32_t itemNum, const Main::Structures::ItemSerialInfo& newItemSerialInfo, std::uint64_t newExpiration)
		{
			if (m_player.replaceItem(itemNum, newItemSerialInfo, newExpiration)) //found)
			{
				m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::replaceItem,
					m_player.getAccountID(), itemNum, newItemSerialInfo, newExpiration);
			}
		}

		const std::unordered_map<std::uint64_t, Main::Structures::EquippedItem>& Session::getEquippedItems() const
		{
			return m_player.getEquippedItems();
		}

		std::uint64_t Session::getTotalEquippedItems() const
		{
			return m_player.getTotalEquippedItems();
		}


		const std::unordered_map<std::uint64_t, Main::Structures::Item>& Session::getItems() const
		{
			return m_player.getItems();
		}

		const std::vector<Main::Structures::Item> Session::getItemsAsVec() const
		{
			return m_player.getItemsAsVec();
		}

		bool Session::deleteItemBasic(const Main::Structures::ItemSerialInfo& itemSerialInfo)
		{
			if (m_player.deleteItemBasic(itemSerialInfo))
			{
				m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::removePlayerItem,
					m_player.getAccountID(), static_cast<std::uint32_t>(itemSerialInfo.itemNumber));
				return true;
			}
			return false;
		}

		void Session::addItems(const std::vector<Item>& items)
		{
			m_player.addItems(items);
			m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::addPlayerItems, m_player.getAccountID(), items, 0);
		}

		void Session::addItem(const Item& item)
		{
			m_player.addItem(item);
			m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::addPlayerItem, item, m_player.getAccountID(), 0);
		}

		void Session::addItems(const std::vector<BoughtItem>& boughtItems)
		{
			auto items = m_player.addItems(boughtItems);
			m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::addPlayerItems, m_player.getAccountID(), items, 0);
		}

		void Session::addItems(const std::vector<Main::Structures::TradeBasicItem> tradedItems)
		{
			auto items = m_player.addItems(tradedItems);
			m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::addPlayerItems, m_player.getAccountID(), items, 0);
		}

		void Session::addItemFromTrade(const Main::Structures::TradeBasicItem& tradeItem)
		{
			auto item = m_player.addItemFromTrade(tradeItem);
			m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::addPlayerItem, item, m_player.getAccountID(), 0);
		}

		void Session::setEquippedItems(const std::unordered_map<std::uint16_t, std::vector<EquippedItem>>& equippedItems)
		{
			m_player.setEquippedItems(equippedItems);
		}

		void Session::setAccountRockTotens(std::uint32_t rt)
		{
			m_player.setAccountRockTotens(rt);
			m_scheduler.addCallback(m_player.getAccountID(), 1, &Main::Persistence::PersistentDatabase::updatePlayerCurrencyByType,
				m_player.getAccountID(), rt, Main::Enums::ItemCurrencyType::ITEM_RT);
		}

		void Session::setAccountMicroPoints(std::uint32_t mp)
		{
			m_player.setAccountMicroPoints(mp);
			m_scheduler.addCallback(m_player.getAccountID(), 2, &Main::Persistence::PersistentDatabase::updatePlayerCurrencyByType,
				m_player.getAccountID(), mp, Main::Enums::ItemCurrencyType::ITEM_MP);
		}

		void Session::setAccountCoins(std::uint16_t coins)
		{
			m_player.setAccountCoins(coins);
			m_scheduler.addCallback(m_player.getAccountID(), 3, &Main::Persistence::PersistentDatabase::updatePlayerCurrencyByType,
				m_player.getAccountID(), coins, Main::Enums::ItemCurrencyType::ITEM_COIN);
		}

		void Session::setAccountLatestCharacterSelected(std::uint16_t latestCharacterSelected)
		{
			m_player.setAccountLatestCharacterSelected(latestCharacterSelected);
			m_scheduler.addCallback(m_player.getAccountID(), 4, &Main::Persistence::PersistentDatabase::updateLatestSelectedCharacter,
				m_player.getAccountID(), latestCharacterSelected);
		}

		void Session::setLevel(std::uint16_t level)
		{
			m_player.setLevel(level);
			m_scheduler.addCallback(m_player.getAccountID(), 5, &Main::Persistence::PersistentDatabase::updatePlayerLevel, m_player.getAccountID(), level);
		}

		void Session::setPlayerName(const char* playerName)
		{
			m_player.setPlayerName(playerName);
			m_scheduler.addCallback(m_player.getAccountID(), 6, &Main::Persistence::PersistentDatabase::updatePlayerName, m_player.getAccountID(), playerName);
		}

		bool Session::addEnergyToItem(const Main::Structures::ItemSerialInfo& itemSerialInfo, std::uint32_t energyAdded)
		{
			auto result = m_player.addEnergyToItem(itemSerialInfo, energyAdded);
			if (result == std::nullopt) return false;
			m_scheduler.addCallback(m_player.getAccountID(), 7, &Main::Persistence::PersistentDatabase::insertEnergyToItem,
				m_player.getAccountID(), static_cast<std::uint32_t>(itemSerialInfo.itemNumber), result->first, static_cast<std::uint32_t>(result->second));
			return true;
		}


		void Session::addFriend(const Main::Structures::Friend& ffriend)
		{
			m_player.addOfflineFriend(ffriend);

			m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::addFriend, m_player.getAccountID(), ffriend.targetAccountId);
			m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::addFriend, ffriend.targetAccountId, m_player.getAccountID());
		}

		bool Session::isFriend(std::uint32_t accountId)
		{
			return m_player.isFriend(accountId);
		}

		void Session::addOnlineFriend(Session* session)
		{
			auto ffriend = m_player.addOnlineFriend(session);
			if (ffriend != std::nullopt)
			{
				m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::addFriend,
					m_player.getAccountID(), ffriend->targetAccountId);
			}
		}

		void Session::equipItem(const std::uint16_t itemNumber)
		{
			m_player.equipItem(itemNumber, m_scheduler);
		}

		void Session::spawnItem(std::uint32_t itemId, const Main::Structures::ItemSerialInfo& itemSerialInfo)
		{
			Common::Network::Packet response;
			response.setTcpHeader(0, Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(66);
			response.setExtra(51);
			response.setMission(0);
			response.setOption(2);
			Main::Structures::SpawnedItem spawnedItem{};
			spawnedItem.itemId = itemId;
			spawnedItem.serialInfo = itemSerialInfo;
			response.setData(reinterpret_cast<std::uint8_t*>(&spawnedItem), sizeof(spawnedItem));
			asyncWrite(response);
			addItem(spawnedItem);
		}

		void Session::spawnItems(const std::vector<Main::Structures::TradeBasicItem>& tradeBasicItems)
		{
			for (const auto& current : tradeBasicItems)
			{
				spawnItem(current.itemId, current.itemSerialInfo);
			}
		}

		bool Session::deleteItem(const Main::Structures::ItemSerialInfo& itemSerialInfoToDelete)
		{
			Common::Network::Packet response;
			response.setTcpHeader(0, Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(89);

			struct ItemDeletion
			{
				std::uint32_t totalItemsToDelete{};
				Main::Structures::ItemSerialInfo itemSerialInfo{};
			} itemDeletion{ 1, itemSerialInfoToDelete };

			response.setData(reinterpret_cast<std::uint8_t*>(&itemDeletion), sizeof(itemDeletion));
			bool removed = m_player.deleteItemBasic(itemSerialInfoToDelete);
			if (removed)
			{
				m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::removePlayerItem,
					m_player.getAccountID(), static_cast<std::uint32_t>(itemSerialInfoToDelete.itemNumber));
			}
			response.setExtra(removed ? 1 : 0);
			asyncWrite(response);
			return removed;
		}

		bool Session::deleteItems(const std::vector<Main::Structures::TradeBasicItem>& tradeBasicItems)
		{
			for (const auto& current : tradeBasicItems)
			{
				if (!deleteItem(current.itemSerialInfo)) return false;
			}
		}

		void Session::temporarilySealAllItems()
		{
			Common::Network::Packet response;
			response.setTcpHeader(0, Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(200);
			response.setExtra(1); // seal success
			std::uint32_t unused = 0;

			// Currently causing a client crash in some special circumstances??
			for (const auto& [itemNum, item] : m_player.getItems())
			{
				std::vector<std::uint8_t> message(24);
				std::memcpy(message.data(), &item.serialInfo, 8); // Item Serial Info
				std::memcpy(message.data() + 8, &unused, 4);
				std::memcpy(message.data() + 12, &unused, 4);
				std::memcpy(message.data() + 16, &item.serialInfo, 8);
				response.setData(message.data(), message.size());
				asyncWrite(response);
			}
		}

		void Session::sendCurrency()
		{
			// NB: Set the currency before using this function
			Common::Network::Packet response;
			response.setTcpHeader(0, Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(307);
			struct Message
			{
				std::uint32_t rt{};
				std::uint32_t mp{};
			};
			const auto& accountInfo = m_player.getAccountInfo();
			Message message{ accountInfo.rockTotens, accountInfo.microPoints };
			response.setData(reinterpret_cast<std::uint8_t*>(&message), sizeof(message));
			asyncWrite(response);
		}

		std::pair<std::array<std::uint32_t, 10>, std::array<std::uint32_t, 7>> Session::getEquippedItemsSeparated() const
		{
			return m_player.getEquippedItemsSeparated();
		}

		void Session::setIsInLobby(bool val)
		{
			m_player.setIsInLobby(val);
		}

		bool Session::isInLobby() const
		{
			return m_player.isInLobby();
		}

		void Session::unsealAllItems()
		{
			Common::Network::Packet response;
			response.setTcpHeader(0, Common::Enums::USER_LARGE_ENCRYPTION);
			response.setOrder(201);
			response.setExtra(1); // unseal success
			std::vector<std::uint8_t> message(8);

			for (const auto& [itemNum, item] : m_player.getItems())
			{
				std::memcpy(message.data(), &item.serialInfo, sizeof(item.serialInfo));
				response.setData(message.data(), message.size());
				asyncWrite(response);
			}
		}

		void Session::unequipItem(uint64_t itemType)
		{
			auto itemNumber = m_player.unequipItem(itemType, m_scheduler);
			if (itemNumber == std::nullopt) return;
			m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::unequipItem, m_player.getAccountID(), *itemNumber);
		}

		bool Session::hasEnoughInventorySpace(std::uint16_t totalNewItems) const
		{
			return m_player.hasEnoughInventorySpace(totalNewItems);
		}

		std::uint64_t Session::getLatestItemNumber() const
		{
			return m_player.getLatestItemNumber();
		}

		void Session::setLatestItemNumber(std::uint64_t itemNum)
		{
			m_player.setLatestItemNumber(itemNum);
		}

		void Session::banAccount(std::uint64_t daysDuration, const std::string& reason, Common::Network::Packet& response)
		{
			using namespace std::chrono;
			using namespace std::literals;
			zoned_time zt{ current_zone(), local_seconds{duration_cast<seconds>(system_clock::now().time_since_epoch()) + seconds(daysDuration * 24 * 60 * 60)} };
			const std::string bannedUntil = std::format("{:%Y-%m-%d %H:%M:%S}", zt.get_sys_time());

			m_scheduler.immediatePersist(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::updateSuspension, m_player.getAccountID(), bannedUntil, reason);

			response.setOrder(73);
			response.setExtra(1);
			response.setData(nullptr, 0);
			asyncWrite(response);
		}

		void Session::muteAccount(std::uint64_t daysDuration, const std::string& reason, const std::string& mutedBy)
		{
			using namespace std::chrono;
			using namespace std::literals;
			zoned_time zt{ current_zone(), local_seconds{duration_cast<seconds>(system_clock::now().time_since_epoch()) + seconds(daysDuration * 24 * 60 * 60)} };
			const std::string mutedUntil = std::format("{:%Y-%m-%d %H:%M:%S}", zt.get_sys_time());
			m_player.mute(reason, mutedBy, mutedUntil);
			m_scheduler.immediatePersist(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::updateMute, m_player.getAccountID(), mutedUntil, reason, mutedBy);
		}

		void Session::unmuteAccount()
		{
			m_player.unmute();
			m_scheduler.immediatePersist(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::unmuteAccount, m_player.getAccountID());
		}

		void Session::setMute(Main::Structures::MuteInfo val)
		{
			val.isMuted ? m_player.mute(val.reason, val.mutedBy, val.mutedUntil) : m_player.unmute();
		}

		Main::Structures::MuteInfo Session::getMuteInfo() const
		{
			return m_player.getMuteInfo();
		}

		bool Session::isMuted() const
		{
			return m_player.isMuted();
		}

		void Session::clear()
		{
			m_scheduler.immediatePersist(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::updateOfflineStatus, m_player.getAccountID());
			m_scheduler.persistFor(m_player.getAccountID());
		}

		void Session::setOnlineDatabaseStatus()
		{
			m_scheduler.immediatePersist(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::updateOnlineStatus, m_player.getAccountID());
		}

		void Session::setPlayerState(Common::Enums::PlayerState playerState)
		{
			m_player.setPlayerState(playerState);
		}

		Common::Enums::PlayerState Session::getPlayerState() const
		{
			return m_player.getPlayerState();
		}

		void Session::lockTrade()
		{
			m_player.lockTrade();
		}

		bool Session::hasPlayerLocked() const
		{
			return m_player.hasPlayerLocked();
		}

		void Session::resetTradeInfo()
		{
			unsealAllItems();
			m_player.resetTradeInfo();
		}

		void Session::addLuckyPoints(std::uint32_t points)
		{
			m_player.addLuckyPoints(points);
		}

		void Session::setLuckyPoints(std::uint32_t points)
		{
			m_player.setLuckyPoints(points);
		}

		std::uint32_t Session::getLuckyPoints() const
		{
			return m_player.getLuckyPoints();
		}

		void Session::setCurrentlyTradingWithAccountId(std::uint32_t targetAccountId)
		{
			m_player.setCurrentlyTradingWithAccountId(targetAccountId);
		}

		std::uint32_t Session::getCurrentlyTradingWithAccountId() const
		{
			return m_player.getCurrentlyTradingWithAccountId();
		}

		void Session::addTradedItem(std::uint32_t itemId, const Main::Structures::ItemSerialInfo& serialInfo)
		{
			m_player.addTradedItem(itemId, serialInfo);
		}

		void Session::removeTradedItem(const Main::Structures::ItemSerialInfo& serialInfo)
		{
			m_player.removeTradedItem(serialInfo);
		}

		void Session::resetTradedItems()
		{
			m_player.resetTradedItems();
		}

		const std::vector<Main::Structures::TradeBasicItem>& Session::getTradedItems() const
		{
			return m_player.getTradedItems();
		}

		void Session::setRoomNumber(std::uint16_t roomNumber)
		{
			m_player.setRoomNumber(roomNumber);
		}

		void Session::leaveRoom()
		{
			m_player.leaveRoom();
		}

		std::uint16_t Session::getRoomNumber() const
		{
			return m_player.getRoomNumber();
		}

		void Session::decreaseRoomNumber()
		{
			m_player.decreaseRoomNumber();
		}

		void Session::setIsInMatch(bool val)
		{
			std::cout << "Set Is In Match set to: " << val << '\n';
			m_player.setIsInMatch(val);
		}

		bool Session::isInMatch() const
		{
			return m_player.isInMatch();
		}

		std::string Session::getPlayerInfoAsString() const
		{
			return m_player.getPlayerInfoAsString();
		}

		void Session::storeEndMatchStats(const Main::Structures::ScoreboardResponse& stats, Main::Enums::MatchEnd matchEnd, bool hasLeveledUp)
		{
			auto previousAccountInfo = m_player.getAccountInfo();
			previousAccountInfo.meleeKills += stats.meleeKills;
			previousAccountInfo.rifleKills += stats.rifleKills;
			previousAccountInfo.shotgunKills += stats.shotgunKills;
			previousAccountInfo.sniperKills += stats.sniperKills;
			previousAccountInfo.microgunKills += stats.mgKills;
			previousAccountInfo.bazookaKills += stats.bazookaKills;
			previousAccountInfo.grenadeKills += stats.grenadeKills;
			previousAccountInfo.killstreak = stats.probablyKillStreak;
			previousAccountInfo.totalKills += stats.totalKills;
			previousAccountInfo.deaths += stats.deaths;
			previousAccountInfo.headshots += stats.headshots;
			//previousAccountInfo.assists += stats.assists;
			previousAccountInfo.experience = stats.newTotalEXP;
			previousAccountInfo.microPoints = stats.newTotalMP;
			if (matchEnd == Main::Enums::MATCH_WON) previousAccountInfo.wins += 1;
			else if (matchEnd == Main::Enums::MATCH_LOST) previousAccountInfo.losses += 1;
			else if (matchEnd == Main::Enums::MATCH_DRAW) previousAccountInfo.draws += 1;
			if (hasLeveledUp) previousAccountInfo.playerLevel += 1;

			m_player.setAccountInfo(previousAccountInfo);
			m_scheduler.addRepetitiveCallback(previousAccountInfo.accountID, &Main::Persistence::PersistentDatabase::updatePlayerStats, previousAccountInfo.accountID,
				previousAccountInfo);

			m_player.storeBatteryObtainedInMatch();
			m_scheduler.addRepetitiveCallback(m_player.getAccountID(), &Main::Persistence::PersistentDatabase::updateBattery, m_player.getAccountID(),
				m_player.getAccountInfo().battery);
		}
	};
}
