
#include "../../include/Classes/Player.h"
#include <unordered_map>
#include <vector>

#include "../../include/Network/MainSession.h"
#include <ConstantDatabase/Structures/SetItemInfo.h>
#include <Utils/Utils.h>

namespace Main
{
	namespace Classes
	{
		using Item = Main::Structures::Item;
		using EquippedItem = Main::Structures::EquippedItem;
		using DetailedEquippedItem = Main::Structures::DetailedEquippedItem;
		using BoughtItem = Main::Structures::BoughtItem;
		using AccountInfo = Main::Structures::AccountInfo;
		using Session = Main::Network::Session;
		using BlockedPlayer = Main::Structures::BlockedPlayer;
		using Friend = Main::Structures::Friend;
		using TradedItem = Main::Structures::TradeBasicItem;
		using Mailbox = Main::Structures::Mailbox;
		using Session = Main::Network::Session;

		Player::Player()
		{
			for (std::uint64_t characterID = 0; characterID <= 8; ++characterID) {
				m_equippedItemByCharacter.emplace(characterID, std::unordered_map<std::uint64_t, EquippedItem>{});
			}
		}

		void Player::setAccountInfo(const AccountInfo& accountInfo)
		{
			m_accountInfo = accountInfo;
		}

		void Player::addBatteryObtainedInMatch(std::uint32_t newBattery)
		{
			m_batteryObtainedInMatch += newBattery;
		}

		void Player::storeBatteryObtainedInMatch()
		{
			std::cout << "Battery Obtained: " << m_batteryObtainedInMatch << '\n';

			if (m_accountInfo.battery + m_batteryObtainedInMatch >= m_accountInfo.maxBattery)
			{
				std::cout << "Max Battery: " << m_accountInfo.maxBattery;
				m_accountInfo.battery = m_accountInfo.maxBattery;
			}
			else
			{
				m_accountInfo.battery += m_batteryObtainedInMatch;
			}
		}

		const AccountInfo& Player::getAccountInfo() const
		{
			return m_accountInfo;
		}

		void Player::setPing(std::uint16_t ping)
		{
			m_ping = ping;
		}

		std::uint16_t Player::getPing() const
		{
			return m_ping;
		}

		std::uint32_t Player::getAccountID() const
		{
			return m_accountInfo.accountID;
		}

		const char* const Player::getPlayerName() const
		{
			return m_accountInfo.nickname;
		}

		void Player::setAccountRockTotens(std::uint32_t rt)
		{
			m_accountInfo.rockTotens = rt;
		}

		void Player::setAccountMicroPoints(std::uint32_t mp)
		{
			m_accountInfo.microPoints = mp;
		}

		void Player::setAccountCoins(std::uint16_t coins)
		{
			m_accountInfo.coins = coins;
		}

		void Player::setAccountLatestCharacterSelected(std::uint16_t latestCharacterSelected)
		{
			m_accountInfo.latestSelectedCharacter = latestCharacterSelected;
		}

		void Player::setLevel(std::uint16_t level)
		{
			m_accountInfo.playerLevel = level + 1;
		}

		void Player::setPlayerName(const char* playerName)
		{
			strncpy(m_accountInfo.nickname, playerName, sizeof(m_accountInfo.nickname));
		}

		bool Player::hasEnoughInventorySpace(std::uint16_t totalNewItems) const
		{
			return (static_cast<std::int32_t>(m_accountInfo.inventorySpace) - m_totalEquippedItems - m_itemsByItemNumber.size()) > totalNewItems;
		}

		void Player::setPlayerState(Common::Enums::PlayerState playerState)
		{
			m_playerState = playerState;
		}

		Common::Enums::PlayerState Player::getPlayerState() const
		{
			return m_playerState;
		}

		void Player::setIsInLobby(bool val)
		{
			m_isInLobby = val;
		}

		bool Player::isInLobby() const
		{
			return m_isInLobby;
		}

		Main::Structures::MuteInfo Player::getMuteInfo() const
		{
			return Main::Structures::MuteInfo{ m_isMuted, m_muteReason, m_mutedBy, m_mutedUntil };
		}

		void Player::mute(const std::string& reason, const std::string& mutedBy, const std::string& mutedUntil)
		{
			m_isMuted = true;
			m_muteReason = reason;
			m_mutedBy = mutedBy;
			m_mutedUntil = mutedUntil;
		}

		void Player::unmute()
		{
			m_isMuted = false;
		}

		bool Player::isMuted() const
		{
			return m_isMuted;
		}

		void Player::addLuckyPoints(std::uint32_t points)
		{
			m_accountInfo.luckyPoints += points;
		}

		void Player::setLuckyPoints(std::uint32_t points)
		{
			m_accountInfo.luckyPoints = points;
		}

		std::uint32_t Player::getLuckyPoints() const
		{
			return static_cast<std::uint32_t>(m_accountInfo.luckyPoints);
		}

		const std::vector<Friend> Player::getFriendlist() const
		{
			std::vector<Friend> ret;
			for (const auto& [ffriend, session] : m_friends)
			{
				ret.push_back(ffriend);
			}
			return ret;
		}

		std::unordered_map<Friend, Session*>& Player::getFriendSessions()
		{
			return m_friends;
		}

		void Player::setFriendList(const std::vector<Friend>& friendlist)
		{
			for (const auto& currentFriend : friendlist)
			{
				m_friends[currentFriend] = nullptr;
			}
		}

		void Player::updateFriend(const Friend& targetFriend, Session* targetSession, bool remove = false)
		{
			if (m_friends.contains(targetFriend)) m_friends.erase(targetFriend);
			m_friends[targetFriend] = remove ? nullptr : targetSession;
		}

		// call once with default "persist", since removeFriend removes the friend for both players
		void Player::deleteFriend(std::uint32_t targetAccountId)
		{
			Main::Structures::Friend toDelete;
			toDelete.targetAccountId = targetAccountId;
			m_friends.erase(toDelete);
		}

		void Player::addOfflineFriend(const Main::Structures::Friend& ffriend)
		{
			m_friends[ffriend] = nullptr;
		}

		std::optional<Main::Structures::Friend> Player::addOnlineFriend(Session* session)
		{
			if (session)
			{
				const auto& accountInfo = session->getAccountInfo();
				Main::Structures::Friend ffriend{ accountInfo.uniqueId, accountInfo.accountID };
				std::memcpy(ffriend.targetNickname, accountInfo.nickname, 16);
				m_friends[ffriend] = session;
				return ffriend;
			}
			return std::nullopt;
		}

		bool Player::isFriend(std::uint32_t accountId)
		{
			for (const auto& [key, val] : m_friends)
			{
				if (key.targetAccountId == accountId)
				{
					return true;
				}
			}
			return false;
		}

		void Player::setUnequippedItems(const std::vector<Item>& items)
		{
			for (const auto& currentItem : items)
			{
				m_latestItemNumber = std::max(m_latestItemNumber, currentItem.serialInfo.itemNumber);
				m_itemsByItemNumber[currentItem.serialInfo.itemNumber] = currentItem;
			}
		}

		const std::optional<Item> Player::findItemBySerialInfo(const Main::Structures::ItemSerialInfo& itemSerialInfo) const
		{
			for (const auto& [itemNumber, item] : m_itemsByItemNumber)
			{
				if (item.serialInfo == itemSerialInfo)
				{
					return item;
				}
			}
			for (const auto& [itemType, item] : m_equippedItemByCharacter.at(m_accountInfo.latestSelectedCharacter)) // m_equippedItemByType)
			{
				if (item.serialInfo == itemSerialInfo)
				{
					return Item{ item };
				}
			}
			return std::nullopt;
		}

		bool Player::replaceItem(const std::uint32_t itemNum, const Main::Structures::ItemSerialInfo& newItemSerialInfo, std::uint64_t newExpiration)
		{
			bool found = false;
			if (m_itemsByItemNumber.contains(itemNum))
			{
				m_itemsByItemNumber[itemNum].serialInfo = newItemSerialInfo;
				found = true;
			}
			else
			{
				for (auto& [itemType, item] : m_equippedItemByCharacter.at(m_accountInfo.latestSelectedCharacter))
				{
					if (item.serialInfo.itemNumber == itemNum)
					{
						item.serialInfo = newItemSerialInfo;
						found = true;
						break;
					}
				}
			}
			return found;
		}

		const std::unordered_map<std::uint64_t, EquippedItem>& Player::getEquippedItems() const
		{
			return m_equippedItemByCharacter.at(m_accountInfo.latestSelectedCharacter);
		}

		const std::unordered_map<std::uint64_t, Item>& Player::getItems() const
		{
			return m_itemsByItemNumber;
		}

		const std::vector<Item> Player::getItemsAsVec() const
		{
			std::vector<Item> items;
			items.reserve(m_itemsByItemNumber.size());
			for (const auto& [unused, item] : m_itemsByItemNumber)
			{
				items.push_back(item);
			}
			return items;
		}

		bool Player::deleteItemBasic(const Main::Structures::ItemSerialInfo& itemSerialInfo)
		{
			for (auto it = m_itemsByItemNumber.begin(); it != m_itemsByItemNumber.end(); ++it)
			{
				if (it->second.serialInfo == itemSerialInfo)
				{
					m_itemsByItemNumber.erase(it);
					return true;
				}
			}
			return false;
		}

		void Player::addItems(const std::vector<Item>& items)
		{
			for (auto& currentItem : items)
			{
				m_itemsByItemNumber[currentItem.serialInfo.itemNumber] = currentItem;
			}
		}

		void Player::addItem(const Item& item)
		{
			m_itemsByItemNumber[item.serialInfo.itemNumber] = item;
		}

		std::vector<Item> Player::addItems(const std::vector<BoughtItem>& boughtItems)
		{
			std::vector<Item> items;
			for (auto& currentItem : boughtItems)
			{
				items.push_back(currentItem);
				m_itemsByItemNumber[currentItem.serialInfo.itemNumber] = currentItem;
			}
			return items;
		}

		std::vector<Main::Structures::Item> Player::addItems(const std::vector<Main::Structures::TradeBasicItem>& tradedItems)
		{
			std::vector<Item> items;
			for (auto& currentItem : tradedItems)
			{
				items.push_back(currentItem);
				m_itemsByItemNumber[currentItem.itemSerialInfo.itemNumber] = currentItem;
			}
			return items;
		}

		Item Player::addItemFromTrade(TradedItem tradeItem)
		{
			tradeItem.itemSerialInfo.itemNumber = ++m_latestItemNumber;
			Main::Structures::Item item{ tradeItem };
			m_itemsByItemNumber[tradeItem.itemSerialInfo.itemNumber] = item;
			return item;
		}

		void Player::setEquippedItems(const std::unordered_map<std::uint16_t, std::vector<EquippedItem>>& equippedItems)
		{
			for (const auto& [characterID, items] : equippedItems)
			{
				for (const auto& currentItem : items)
				{
					m_latestItemNumber = std::max(m_latestItemNumber, currentItem.serialInfo.itemNumber);
					auto& itemMap = m_equippedItemByCharacter[characterID];
					itemMap[currentItem.type >= 17 ? Common::Enums::SET : currentItem.type] = currentItem;
					++m_totalEquippedItems;
				}
			}
		}

		std::optional<std::pair<std::uint16_t, std::uint64_t>>
			Player::addEnergyToItem(const Main::Structures::ItemSerialInfo& itemSerialInfo, std::uint32_t energyAdded)
		{
			for (auto& [itemNumber, item] : m_itemsByItemNumber)
			{
				if (item.serialInfo == itemSerialInfo)
				{
					item.energy += energyAdded;
					m_accountInfo.battery -= energyAdded;
					return std::pair{ item.energy, (uint64_t)m_accountInfo.battery };
				}
			}
			for (auto& [itemType, item] : m_equippedItemByCharacter.at(m_accountInfo.latestSelectedCharacter))
			{
				if (item.serialInfo == itemSerialInfo)
				{
					item.energy += energyAdded;
					m_accountInfo.battery -= energyAdded;
					return std::pair{ item.energy, (uint64_t)m_accountInfo.battery };
				}
			}
			return std::nullopt;
		}

		void Player::unequipItemImpl(std::uint64_t itemType, Main::Persistence::MainScheduler& scheduler)
		{
			auto& itemMap = m_equippedItemByCharacter[m_accountInfo.latestSelectedCharacter];
			if (!itemMap.contains(itemType)) return;

			auto itemNumber = itemMap.at(itemType).serialInfo.itemNumber;
			m_itemsByItemNumber[itemNumber] = Item{ itemMap.at(itemType) };
			itemMap.erase(itemType);
			--m_totalEquippedItems;

			std::cout << "Item Unequipped with Type: " << itemType << '\n';

			scheduler.addRepetitiveCallback(m_accountInfo.accountID, &Main::Persistence::PersistentDatabase::unequipItem,
				m_accountInfo.accountID, static_cast<std::uint64_t>(itemNumber));
		}

		// ugly design but easier to write, ideally we shouldn't pass the scheduler to this function...
		// TODO: Test this, add logs, check si_acce_B and si_acce_C
		void Player::equipItem(const std::uint16_t itemNumber, Main::Persistence::MainScheduler& scheduler)
		{
			// Find the type of the to-be-added equipped item
			EquippedItem equippedItem = EquippedItem{ m_itemsByItemNumber[itemNumber] };

			// Special case: the user is equipping a set => we need to unequip item types that are already present in said set.
			if (equippedItem.type == Common::Enums::ItemType::SET)
			{
				std::cout << "User Equipping Set. Unequipping Parts Relative To Set...\n";
				using setItems = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::SetItemInfo>;
				const auto entry = setItems::getInstance().getEntry("si_id", m_itemsByItemNumber[itemNumber].id);

				if (entry != std::nullopt)
				{
					for (auto currentTypeNotNull : Common::Utils::getPartTypesWhereSetItemInfoTypeNotNull(*entry))
					{
						std::cout << "Set Part Found. Unequipping (Type: " << currentTypeNotNull << ")\n";
						unequipItemImpl(currentTypeNotNull, scheduler);
					}
				}
			}

			auto& itemMap = m_equippedItemByCharacter[m_accountInfo.latestSelectedCharacter];

			// Again, special case: the user has an already equipped set. We need to unequip it if the user is now equipping a part type that is already present in said set.
			if (itemMap.contains(Common::Enums::ItemType::SET))
			{
				std::cout << "User has a Set Equipped. Unequipping the set before equipping the parts...\n";
				using setItems = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::SetItemInfo>;
				const auto entry = setItems::getInstance().getEntry("si_id", itemMap[Common::Enums::ItemType::SET].id >> 1);

				if (entry != std::nullopt)
				{
					for (auto currentTypeNotNull : Common::Utils::getPartTypesWhereSetItemInfoTypeNotNull(*entry))
					{
						if (equippedItem.type == currentTypeNotNull)
						{
							std::cout << "Set Found. Unequipping...\n";
							unequipItemImpl(Common::Enums::SET, scheduler);
							break;
						}
					}
				}
			}

			// Delete this item from the Non-Equipped items, since we'll add it to the Equipped items.
			m_itemsByItemNumber.erase(itemNumber);

			// now proceed normally: check if equippedItems already has such a type
			if (itemMap.contains(equippedItem.type))
			{
				std::cout << "Unequipping Item...\n";

				auto toUnequipItemNumber = itemMap.at(equippedItem.type).serialInfo.itemNumber;
				m_itemsByItemNumber[toUnequipItemNumber] = Item{ itemMap.at(equippedItem.type) };
				itemMap.erase(equippedItem.type);
				itemMap[equippedItem.type] = equippedItem;

				scheduler.addRepetitiveCallback(m_accountInfo.accountID, &Main::Persistence::PersistentDatabase::swapItems,
					m_accountInfo.accountID, toUnequipItemNumber, static_cast<std::uint64_t>(equippedItem.serialInfo.itemNumber),
					static_cast<std::uint16_t>(m_accountInfo.latestSelectedCharacter));
				return;
			}

			// Otherwise just add the to-be-added item to the equipped items.
			itemMap[equippedItem.type] = equippedItem;
			++m_totalEquippedItems;

			scheduler.addRepetitiveCallback(m_accountInfo.accountID, &Main::Persistence::PersistentDatabase::equipItem,
				m_accountInfo.accountID, static_cast<std::uint64_t>(equippedItem.serialInfo.itemNumber), static_cast<std::uint16_t>(m_accountInfo.latestSelectedCharacter));
		}

		std::optional<std::uint64_t> Player::unequipItem(std::uint64_t itemType, Main::Persistence::MainScheduler& scheduler)
		{
			auto& itemMap = m_equippedItemByCharacter[m_accountInfo.latestSelectedCharacter];

			if (!itemMap.contains(itemType))
			{
				std::cout << "Item NOT unequipped!\n";
				return std::nullopt;
			}
			std::uint64_t itemNumber = itemMap.at(itemType).serialInfo.itemNumber;

			m_itemsByItemNumber[itemNumber] = Item{ itemMap.at(itemType) };
			itemMap.erase(itemType);
			--m_totalEquippedItems;
			std::cout << "Item Unequipped\n";
			return itemNumber;
		}

		std::uint64_t Player::getTotalEquippedItems() const
		{
			return m_totalEquippedItems;
		}

		std::uint64_t Player::getLatestItemNumber() const
		{
			return m_latestItemNumber;
		}

		void Player::setLatestItemNumber(std::uint64_t itemNum)
		{
			m_latestItemNumber = itemNum;
		}

		std::pair<std::array<std::uint32_t, 10>, std::array<std::uint32_t, 7>> Player::getEquippedItemsSeparated() const
		{
			std::array<std::uint32_t, 10> equippedPlayerItems{};
			std::array<std::uint32_t, 7> equippedPlayerWeapons{};

			auto& itemMap = m_equippedItemByCharacter.at(m_accountInfo.latestSelectedCharacter);

			// Set is a special case
			if (itemMap.contains(Common::Enums::SET))
			{
				using setItems = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::SetItemInfo>;
				const auto& setItemId = itemMap.at(Common::Enums::SET).id >> 1;
				auto entry = setItems::getInstance().getEntry("si_id", setItemId);
				if (entry != std::nullopt)
				{
					for (const auto& currentTypeNotNull : Common::Utils::getPartTypesWhereSetItemInfoTypeNotNull(*entry))
					{
						equippedPlayerItems[currentTypeNotNull] = setItemId;
					}
				}
			}
			for (std::size_t i = 0; i < equippedPlayerItems.size(); ++i)
			{
				if (itemMap.contains(i))
				{
					equippedPlayerItems[i] = itemMap.at(i).id >> 1;
				}
			}
			for (std::size_t i = 0; i < equippedPlayerWeapons.size(); ++i)
			{
				if (itemMap.contains(i + 10))
				{
					equippedPlayerWeapons[i] = itemMap.at(i + 10).id >> 1;
				}
			}
			return std::pair{ equippedPlayerItems, equippedPlayerWeapons };
		}

		void Player::blockAccount(std::uint32_t accountId, const char* nickname)
		{
			Main::Structures::BlockedPlayer blocked{ accountId };
			std::memcpy(blocked.targetNickname, nickname, 16);

			m_blockedAccounts.push_back(blocked);
		}

		bool Player::unblockAccount(std::uint32_t accountId)
		{
			for (auto it = m_blockedAccounts.begin(); it != m_blockedAccounts.end(); ++it)
			{
				if (it->targetAccountId == accountId)
				{
					m_blockedAccounts.erase(it);
					return true;
				}
			}
			return false;
		}

		bool Player::hasBlocked(std::uint32_t accountId) const
		{
			for (const auto& currentBlocked : m_blockedAccounts)
			{
				if (currentBlocked.targetAccountId == accountId)
				{
					return true;
				}
			}
			return false;
		}

		const std::vector<Main::Structures::BlockedPlayer>& Player::getBlockedPlayers() const
		{
			return m_blockedAccounts;
		}

		void Player::setBlockedPlayers(const std::vector<Main::Structures::BlockedPlayer>& blockedPlayers)
		{
			m_blockedAccounts = blockedPlayers;
		}


		// Trade system
		void Player::lockTrade()
		{
			m_hasPlayerLocked = true;
		}

		bool Player::hasPlayerLocked() const
		{
			return m_hasPlayerLocked;
		}

		void Player::resetTradeInfo()
		{
			m_hasPlayerLocked = false;
			m_tradedItems.clear();
			m_currentlyTradingWithAccountId = 0;
			setPlayerState(Common::Enums::PlayerState::STATE_INVENTORY);
		}

		void Player::setCurrentlyTradingWithAccountId(std::uint32_t targetAccountId)
		{
			m_currentlyTradingWithAccountId = targetAccountId;
		}

		std::uint32_t Player::getCurrentlyTradingWithAccountId() const
		{
			return m_currentlyTradingWithAccountId;
		}

		void Player::addTradedItem(std::uint32_t itemId, const Main::Structures::ItemSerialInfo& serialInfo)
		{
			m_tradedItems.push_back(Main::Structures::TradeBasicItem{ itemId , serialInfo });
		}

		void Player::removeTradedItem(const Main::Structures::ItemSerialInfo& serialInfo)
		{
			for (auto it = m_tradedItems.begin(); it != m_tradedItems.end(); ++it)
			{
				if (it->itemSerialInfo == serialInfo)
				{
					m_tradedItems.erase(it);
					return;
				}
			}
		}

		void Player::resetTradedItems()
		{
			m_tradedItems.clear();
		}

		const std::vector<TradedItem>& Player::getTradedItems() const
		{
			return m_tradedItems;
		}


		// Mailbox
		void Player::addMailboxReceived(const Main::Structures::Mailbox& mailbox)
		{
			m_mailboxReceived.push_back(mailbox);
		}

		void Player::addMailboxSent(const Main::Structures::Mailbox& mailbox)
		{
			m_mailboxSent.push_back(mailbox);
		}

		bool Player::deleteSentMailbox(std::uint32_t timestamp, std::uint32_t accountId)
		{
			for (auto it = m_mailboxSent.begin(); it != m_mailboxSent.end(); ++it)
			{
				if (it->accountId == accountId && it->timestamp == timestamp)
				{
					m_mailboxSent.erase(it);
					return true;
				}
			}
			return false;
		}

		bool Player::deleteReceivedMailbox(std::uint32_t timestamp, std::uint32_t accountId)
		{
			for (auto it = m_mailboxReceived.begin(); it != m_mailboxReceived.end(); ++it)
			{
				if (it->accountId == accountId && it->timestamp == timestamp)
				{
					m_mailboxReceived.erase(it);
					return true;
				}
			}
			return false;
		}

		const std::vector<Main::Structures::Mailbox>& Player::getMailboxReceived() const
		{
			return m_mailboxReceived;
		}

		const std::vector<Main::Structures::Mailbox>& Player::getMailboxSent() const
		{
			return m_mailboxSent;
		}

		void Player::setMailbox(const std::vector<Main::Structures::Mailbox>& mailbox, bool sent)
		{
			if (sent) m_mailboxSent = mailbox;
			else m_mailboxReceived = mailbox;
		}


		// Room info
		void Player::setRoomNumber(std::uint16_t roomNumber)
		{
			m_roomNumber = roomNumber;
		}

		std::uint16_t Player::getRoomNumber() const
		{
			return m_roomNumber;
		}

		void Player::setIsInMatch(bool val)
		{
			std::cout << "Set Is In Match to: " << val << '\n';
			m_isInMatch = val;
		}

		bool Player::isInMatch() const
		{
			return m_isInMatch;
		}

		void Player::leaveRoom()
		{
			std::cout << "Player left match\n";
			setRoomNumber(0);
			setPlayerState(Common::Enums::STATE_LOBBY);
		    setIsInLobby(true);
			setIsInMatch(false);
			m_batteryObtainedInMatch = 0;
		}

		void Player::decreaseRoomNumber()
		{
			if (m_roomNumber > 0)
			{
				--m_roomNumber;
				std::cout << "PLAYER ROOM NUMBER: " << m_roomNumber << '\n';
			}
		}
	}
}
