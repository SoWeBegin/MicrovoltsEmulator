#ifndef MAIN_SESSION_HEADER
#define MAIN_SESSION_HEADER

#include "Network/Session.h"
#include "../Structures/AccountInfo/MainAccountInfo.h"
#include "../Structures/Item/MainItem.h"
#include "../Structures/Item/MainEquippedItem.h"
#include "../Persistence/MainScheduler.h"
#include "../Persistence/MainDatabaseManager.h"
#include "../Structures/PlayerLists/BlockedPlayer.h"
#include "../Structures/PlayerLists/Friend.h"
#include "../Structures/TradeSystem/TradeSystemItem.h"
#include "../Structures/Mailbox.h"
#include "Enums/GameEnums.h"

#include "../Classes/Player.h"
#include <functional>
#include <chrono>
#include <asio.hpp>
#include "../Structures/EndScoreboard.h"

namespace Main
{
	namespace Network
	{
		class Session : public Common::Network::Session
		{
		public:
			using Item = Main::Structures::Item;
			using EquippedItem = Main::Structures::EquippedItem;
			using DetailedEquippedItem = Main::Structures::DetailedEquippedItem;
			using BoughtItem = Main::Structures::BoughtItem;
			using AccountInfo = Main::Structures::AccountInfo;
			using tcp = asio::ip::tcp;

			Main::Classes::Player m_player{};
			Main::Persistence::MainScheduler& m_scheduler;

		public:
			explicit Session(Main::Persistence::MainScheduler& scheduler, tcp::socket&& socket, std::function<void(std::size_t)> fnct);

			std::size_t getSessionId() const;

			void onPacket(std::vector<std::uint8_t>& data) override;

			void addMailboxReceived(const Main::Structures::Mailbox& mailbox);

			void addMailboxSent(const Main::Structures::Mailbox& mailbox);

			bool deleteSentMailbox(std::uint32_t timestamp, std::uint32_t accountId);

			bool deleteReceivedMailbox(std::uint32_t timestamp, std::uint32_t accountId);

			const std::vector<Main::Structures::Mailbox>& getMailboxReceived() const;

			const std::vector<Main::Structures::Mailbox>& getMailboxSent() const;

			void setMailbox(const std::vector<Main::Structures::Mailbox>& mailbox, bool sent);

			void setAccountInfo(const AccountInfo& accountInfo);

			void addBatteryObtainedInMatch(std::uint32_t newBattery);

			const AccountInfo& getAccountInfo() const;

			const std::vector<Main::Structures::Friend> getFriendlist() const;

			std::unordered_map<Main::Structures::Friend, Session*>& getFriendSessions();

			const char* const getPlayerName() const;

			void sendAccountInfo(Common::Network::Packet& response);

			void setPing(std::uint16_t ping);

			std::uint16_t getPing() const;

			void setUnequippedItems(const std::vector<Item>& items);

			void setFriendList(const std::vector<Main::Structures::Friend>& friendlist);

			void logFriend(std::uint8_t logType, std::uint32_t targetAccountId);
			
			void updateFriendSession(Session* targetSession, bool remove = false);

			void blockAccount(std::uint32_t accountId, const char* nickname);

			bool unblockAccount(std::uint32_t accountId);

			bool hasBlocked(std::uint32_t accountId) const;

			const std::vector<Main::Structures::BlockedPlayer>& getBlockedPlayers() const;

			void setBlockedPlayers(const std::vector<Main::Structures::BlockedPlayer>& blockedPlayers);

			// call once with default "persist", since removeFriend removes the friend for both players
			void deleteFriend(std::uint32_t targetAccountId, bool persist = true);

			const std::optional<Item> findItemBySerialInfo(const Main::Structures::ItemSerialInfo& itemSerialInfo) const;

			void replaceItem(const std::uint32_t itemNum, const Main::Structures::ItemSerialInfo& newItemSerialInfo, std::uint64_t newExpiration);

			const std::unordered_map<std::uint64_t, EquippedItem>& getEquippedItems() const;

			std::uint64_t getTotalEquippedItems() const;

			const std::unordered_map<std::uint64_t, Main::Structures::Item>& getItems() const;

			const std::vector<Item> getItemsAsVec() const;

			bool deleteItemBasic(const Main::Structures::ItemSerialInfo& itemSerialInfo);

			void addItems(const std::vector<Item>& items);

			void addItem(const Item& item);

			void addItems(const std::vector<BoughtItem>& boughtItems);

			void addItems(const std::vector<Main::Structures::TradeBasicItem> tradedItems);

			void addItemFromTrade(const Main::Structures::TradeBasicItem& tradeItem);

			void setEquippedItems(const std::unordered_map<std::uint16_t, std::vector<EquippedItem>>& equippedItems);

			void setAccountRockTotens(std::uint32_t rt);

			void setAccountMicroPoints(std::uint32_t mp);

			void setAccountCoins(std::uint16_t coins);

			void setAccountLatestCharacterSelected(std::uint16_t latestCharacterSelected);

			void setLevel(std::uint16_t level);

			void setPlayerName(const char* playerName);

			bool addEnergyToItem(const Main::Structures::ItemSerialInfo& itemSerialInfo, std::uint32_t energyAdded);


			void addFriend(const Main::Structures::Friend& ffriend);

			bool isFriend(std::uint32_t accountId);

			void addOnlineFriend(Session* session);

			void equipItem(const std::uint16_t itemNumber);

			void spawnItem(std::uint32_t itemId, const Main::Structures::ItemSerialInfo& itemSerialInfo);

			void spawnItems(const std::vector<Main::Structures::TradeBasicItem>& tradeBasicItems);

			bool deleteItem(const Main::Structures::ItemSerialInfo& itemSerialInfoToDelete);

			bool deleteItems(const std::vector<Main::Structures::TradeBasicItem>& tradeBasicItems);

			void temporarilySealAllItems();

			void sendCurrency();

			std::pair<std::array<std::uint32_t, 10>, std::array<std::uint32_t, 7>> getEquippedItemsSeparated() const;

			void setIsInLobby(bool val);

			bool isInLobby() const;

			void unsealAllItems();

			void unequipItem(uint64_t itemType);

			bool hasEnoughInventorySpace(std::uint16_t totalNewItems) const;

			std::uint64_t getLatestItemNumber() const;

			void setLatestItemNumber(std::uint64_t itemNum);

			void banAccount(std::uint64_t daysDuration, const std::string& reason, Common::Network::Packet& response);

			void muteAccount(std::uint64_t daysDuration, const std::string& reason, const std::string& mutedBy);

			void unmuteAccount();

			void setMute(Main::Structures::MuteInfo val);

			Main::Structures::MuteInfo getMuteInfo() const;

			bool isMuted() const;

			void clear();

			void setOnlineDatabaseStatus();

			void setPlayerState(Common::Enums::PlayerState playerState);

			Common::Enums::PlayerState getPlayerState() const;

			void lockTrade();

			bool hasPlayerLocked() const;

			void resetTradeInfo();

			void addLuckyPoints(std::uint32_t points);

			void setLuckyPoints(std::uint32_t points);

			std::uint32_t getLuckyPoints() const;

			void setCurrentlyTradingWithAccountId(std::uint32_t targetAccountId);

			std::uint32_t getCurrentlyTradingWithAccountId() const;

			void addTradedItem(std::uint32_t itemId, const Main::Structures::ItemSerialInfo& serialInfo);

			void removeTradedItem(const Main::Structures::ItemSerialInfo& serialInfo);

			void resetTradedItems();

			const std::vector<Main::Structures::TradeBasicItem>& getTradedItems() const;

			void setRoomNumber(std::uint16_t roomNumber);

			void leaveRoom();

			std::uint16_t getRoomNumber() const;

			void decreaseRoomNumber();

			void setIsInMatch(bool val);

			bool isInMatch() const;

			std::string getPlayerInfoAsString() const;

			void storeEndMatchStats(const Main::Structures::ScoreboardResponse& stats, Main::Enums::MatchEnd matchEnd, bool hasLeveledUp);
		};
	}
}
#endif