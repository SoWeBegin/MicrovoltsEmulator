#ifndef PLAYER_CLASS_H
#define PLAYER_CLASS_H

#include "../Structures/Item/MainEquippedItem.h"
#include "../Structures/Item/MainBoughtItem.h"
#include "../Structures/AccountInfo/MainAccountInfo.h"
#include "../Structures/PlayerLists/Friend.h"
#include "../Structures/Mailbox.h"
#include "../Structures/PlayerLists/BlockedPlayer.h"
#include "../Structures/Item/MainItem.h"
#include "../Persistence/MainScheduler.h"

#include <unordered_map>
#include <vector>
#include <array>

namespace Common { namespace Network { class Session; } }
namespace Main { namespace Network { class Session; } }
namespace Main
{
	namespace Classes
	{
		class Player
		{
		private:
			using Item = Main::Structures::Item;
			using EquippedItem = Main::Structures::EquippedItem;
			using DetailedEquippedItem = Main::Structures::DetailedEquippedItem;
			using BoughtItem = Main::Structures::BoughtItem;
			using AccountInfo = Main::Structures::AccountInfo;
			using Session = Main::Network::Session;
			using BlockedPlayer = Main::Structures::BlockedPlayer;
			using Friend = Main::Structures::Friend;
			using Mailbox = Main::Structures::Mailbox;
			using Session = Main::Network::Session;

			AccountInfo m_accountInfo{};
			std::unordered_map<std::uint64_t, Item> m_itemsByItemNumber{};
			std::unordered_map<std::uint64_t, std::unordered_map<std::uint64_t, EquippedItem>> m_equippedItemByCharacter{}; // [character][itemType] = [equippedItem]
			std::uint64_t m_totalEquippedItems{};
			std::unordered_map<Friend, Session*> m_friends{};
			std::vector<BlockedPlayer> m_blockedAccounts{};
			Common::Enums::PlayerState m_playerState{};
			std::uint16_t m_ping{};
			bool m_isMuted{ false };
			std::string m_mutedBy;
			std::string m_muteReason;
			std::string m_mutedUntil;
			bool m_isInLobby{ true };

			// Mailbox specific
			std::vector<Mailbox> m_mailboxReceived{};
			std::vector<Mailbox> m_mailboxSent{};

			// Other
			std::uint16_t m_roomNumber{};
			bool m_isInMatch{};
			mutable std::uint64_t m_latestItemNumber{};
			std::uint32_t m_batteryObtainedInMatch{};


		public:
			Player();

			// Account info
			void setAccountInfo(const AccountInfo& accountInfo);
			void addBatteryObtainedInMatch(std::uint32_t newBattery);
			void storeBatteryObtainedInMatch();
			const AccountInfo& getAccountInfo() const;
			std::uint32_t getAccountID() const;
			const char* const getPlayerName() const;
			void setAccountRockTotens(std::uint32_t rt);
			void setAccountMicroPoints(std::uint32_t mp);
			void setAccountCoins(std::uint16_t coins);
			void setAccountLatestCharacterSelected(std::uint16_t latestCharacterSelected);
			void setLevel(std::uint16_t level);
			void setPlayerName(const char* playerName);
			bool hasEnoughInventorySpace(std::uint16_t totalNewItems) const;
			void setPlayerState(Common::Enums::PlayerState playerState);
			Common::Enums::PlayerState getPlayerState() const;
			void addLuckyPoints(std::uint32_t points);
			void setLuckyPoints(std::uint32_t points);
			std::uint32_t getLuckyPoints() const;
			void setPing(std::uint16_t ping);
			std::uint16_t getPing() const;
			void setIsInLobby(bool val);
			bool isInLobby() const;
			void mute(const std::string& reason, const std::string& mutedBy, const std::string& mutedUntil);
			void unmute();
			Main::Structures::MuteInfo getMuteInfo() const;
			bool isMuted() const;

			// Friends
			const std::vector<Friend> getFriendlist() const;
			std::unordered_map<Friend, Session*>& getFriendSessions();
			void setFriendList(const std::vector<Friend>& friendlist);
			void updateFriend(const Friend& targetFriend, Session* targetSession, bool remove);
			// call once with default "persist", since removeFriend removes the friend for both players
			void deleteFriend(std::uint32_t targetAccountId);
			void addOfflineFriend(const Main::Structures::Friend& ffriend);
			std::optional<Main::Structures::Friend> addOnlineFriend(Session* session);
			bool isFriend(std::uint32_t accountId);

			// Player items
			void setUnequippedItems(const std::vector<Item>& items);
			const std::optional<Item> findItemBySerialInfo(const Main::Structures::ItemSerialInfo& itemSerialInfo) const;
			bool replaceItem(const std::uint32_t itemNum, const Main::Structures::ItemSerialInfo& newItemSerialInfo, std::uint64_t newExpiration);
			const std::unordered_map<std::uint64_t, EquippedItem>& getEquippedItems() const;
			const std::unordered_map<std::uint64_t, Item>& getItems() const;
			const std::vector<Item> getItemsAsVec() const;
			bool deleteItemBasic(const Main::Structures::ItemSerialInfo& itemSerialInfo);
			void addItems(const std::vector<Item>& items);
			void addItem(const Item& item);
			std::vector<Item> addItems(const std::vector<BoughtItem>& boughtItems);
			void setEquippedItems(const std::unordered_map<std::uint16_t, std::vector<EquippedItem>>& equippedItems);
			std::optional<std::pair<std::uint16_t, std::uint64_t>>
			addEnergyToItem(const Main::Structures::ItemSerialInfo& itemSerialInfo, std::uint32_t energyAdded);
			// ugly design but easier to write, ideally we shouldn't pass the scheduler to this function...
			void equipItem(const std::uint16_t itemNumber, Main::Persistence::MainScheduler& scheduler, std::uint32_t character = -1);
			std::optional<std::uint64_t> unequipItem(uint64_t itemType, Main::Persistence::MainScheduler& scheduler);
			std::uint64_t getTotalEquippedItems() const;
			std::uint32_t addBattery(std::uint32_t battery);

			void unequipItemImpl(std::uint64_t itemType, Main::Persistence::MainScheduler& scheduler, std::uint32_t character = -1);

			std::uint64_t getLatestItemNumber() const;
			void setLatestItemNumber(std::uint64_t itemNum);
			std::pair<std::array<std::uint32_t, 10>, std::array<std::uint32_t, 7>> getEquippedItemsSeparated() const;
			bool unequipItemIfEquipped(std::uint64_t itemNumber, std::uint32_t characterId, Main::Persistence::MainScheduler& scheduler);
			void equipItemIfNotEquipped(std::uint64_t itemNumber, std::uint32_t characterId, Main::Persistence::MainScheduler& scheduler);

			// Blocked players
			void blockAccount(std::uint32_t accountId, const char* nickname);
			bool unblockAccount(std::uint32_t accountId);
			bool hasBlocked(std::uint32_t accountId) const;
			const std::vector<Main::Structures::BlockedPlayer>& getBlockedPlayers() const;
			void setBlockedPlayers(const std::vector<Main::Structures::BlockedPlayer>& blockedPlayers);

			// Mailbox
			void addMailboxReceived(const Main::Structures::Mailbox& mailbox);
			void addMailboxSent(const Main::Structures::Mailbox& mailbox);
			bool deleteSentMailbox(std::uint32_t timestamp);
			bool deleteReceivedMailbox(std::uint32_t timestamp);
			const std::vector<Main::Structures::Mailbox>& getMailboxReceived() const;
			const std::vector<Main::Structures::Mailbox>& getMailboxSent() const;
			void setMailbox(const std::vector<Main::Structures::Mailbox>& mailbox, bool sent);

			// Room info
			void setRoomNumber(std::uint16_t roomNumber);
			std::uint16_t getRoomNumber() const;
			void decreaseRoomNumber();
			void setIsInMatch(bool val);
			bool isInMatch() const;
			void leaveRoom();

			std::string getPlayerInfoAsString() const
			{
				return "(PlayerName: " + std::string(m_accountInfo.nickname) + ", RoomNumber: " + std::to_string(m_roomNumber) + ", IsInMatch : " + std::to_string(m_isInMatch)
					+ ", IsInLobby: " + std::to_string(m_isInLobby) + ")";
			}
		};
	}
}

#endif
