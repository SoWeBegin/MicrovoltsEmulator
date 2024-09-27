#ifndef MAIN_DATABASE_MANAGER_H
#define MAIN_DATABASE_MANAGER_H

#include <string>
#include <iostream>

#include "../MainEnums.h"
#define SQLITECPP_COMPILE_DLL
#include "../../ExternalLibraries/SQLiteCpp/SQLiteCpp.h"
#include "../Structures/AccountInfo/MainAccountInfo.h"
#include "../Structures/Item/MainBoughtItem.h"
#include "../Structures/Item/MainEquippedItem.h"
#include "../CdbUtils.h"
#include "../Structures/MainEventsList.h"
#include "../Structures/PlayerLists/Friend.h"
#include "../Structures/PlayerLists/BlockedPlayer.h"
#include "../Structures/Mailbox.h"
#include "../Structures/AccountInfo/MuteInfo.h"


namespace Main
{
	namespace Persistence
	{
		class PersistentDatabase
		{
		private:
			SQLite::Database db;

			using Item = Main::Structures::Item;
			using BoughtItem = Main::Structures::BoughtItem;
			using EquippedItem = Main::Structures::EquippedItem;
			using DetailedEquippedItem = Main::Structures::DetailedEquippedItem;

		public:
			PersistentDatabase(const std::string& path)
				: db{ SQLite::Database(path.c_str(), SQLite::OPEN_READWRITE) }
			{
			}

			void updatePlayerCurrencyByType(std::uint32_t accountID, std::uint32_t newAmount, Main::Enums::ItemCurrencyType currencyType)
			{
				try
				{
					std::string sql;
					if (currencyType == Main::Enums::ITEM_MP) sql = "UPDATE Users SET MicroPoints = ? WHERE AccountID = ?";
					else if (currencyType == Main::Enums::ITEM_RT) sql = "UPDATE Users SET RockTotens = ? WHERE AccountID = ?";
					else if (currencyType == Main::Enums::ITEM_COUPON) { /* TODO */ }
					else if (currencyType == Main::Enums::ITEM_COIN)  sql = "UPDATE Users SET Coins = ? WHERE AccountID = ?";
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, sql);
					query.bind(1, newAmount);
					query.bind(2, accountID);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::removeCurrency] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::removeCurrency] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::removeCurrency] SQLite exception: " << e.what() << '\n';
				}
			}

			void updateLatestSelectedCharacter(std::uint32_t accountID, std::uint16_t characterId)
			{
				try
				{
					std::string updateCharacterQuery = "UPDATE Users SET LastCharacterUsed = ? WHERE AccountID = ?";
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, updateCharacterQuery);
					query.bind(1, characterId);
					query.bind(2, accountID);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::updateLatestSelectedCharacter] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::updateLatestSelectedCharacter] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::updateLatestSelectedCharacter] SQLite exception: " << e.what() << '\n';
				}
			}

			Main::Structures::AccountInfo getPlayerInfo(std::uint32_t playerID)
			{
				Main::Structures::AccountInfo playerInfoStructure{};
				try
				{
					SQLite::Statement query(db, "SELECT Users.*, Clans.Clanname, Clans.ClanFrontIcon, Clans.ClanBackIcon FROM Users LEFT JOIN Clans ON Users.ClanID = Clans.ClanId WHERE AccountID = ?");
					query.bind(1, playerID);

					if (query.executeStep())
					{
						strcpy_s(playerInfoStructure.nickname, sizeof(playerInfoStructure.nickname), query.getColumn("Nickname").getString().c_str());
						strcpy_s(playerInfoStructure.clanName, sizeof(playerInfoStructure.clanName), query.getColumn("Clanname").getString().c_str());

						playerInfoStructure.accountID = playerID;
						playerInfoStructure.accountKey = static_cast<std::uint32_t>(query.getColumn("AccountKey").getInt());
						playerInfoStructure.totalKills = static_cast<std::uint32_t>(query.getColumn("Kills").getInt());
						playerInfoStructure.deaths = static_cast<std::uint32_t>(query.getColumn("Deaths").getInt());
						playerInfoStructure.wins = static_cast<std::uint32_t>(query.getColumn("Wins").getInt());
						playerInfoStructure.losses = static_cast<std::uint32_t>(query.getColumn("Loses").getInt());
						playerInfoStructure.draws = static_cast<std::uint32_t>(query.getColumn("Draws").getInt());
						playerInfoStructure.meleeKills = static_cast<std::uint32_t>(query.getColumn("MeleeKills").getInt());
						playerInfoStructure.rifleKills = static_cast<std::uint32_t>(query.getColumn("RifleKills").getInt());
						playerInfoStructure.shotgunKills = static_cast<std::uint32_t>(query.getColumn("ShotgunKills").getInt());
						playerInfoStructure.sniperKills = static_cast<std::uint32_t>(query.getColumn("SniperKills").getInt());
						playerInfoStructure.microgunKills = static_cast<std::uint32_t>(query.getColumn("GatlingKills").getInt());
						playerInfoStructure.bazookaKills = static_cast<std::uint32_t>(query.getColumn("BazookaKills").getInt());
						playerInfoStructure.grenadeKills = static_cast<std::uint32_t>(query.getColumn("GrenadeKills").getInt());
						playerInfoStructure.killstreak = static_cast<std::uint64_t>(query.getColumn("HighestKillstreak").getInt());
						playerInfoStructure.headshots = static_cast<std::uint64_t>(query.getColumn("Headshots").getInt());
						playerInfoStructure.playtime = static_cast<std::uint32_t>(query.getColumn("Playtime").getInt());
						playerInfoStructure.clanId = static_cast<std::uint32_t>(query.getColumn("ClanID").getInt());
						playerInfoStructure.latestSelectedCharacter = static_cast<std::uint64_t>(query.getColumn("LastCharacterUsed").getInt());
						playerInfoStructure.playerLevel = static_cast<std::uint64_t>(query.getColumn("Level").getInt()) + 1;
						playerInfoStructure.battery = static_cast<std::uint64_t>(query.getColumn("Battery").getInt());
						playerInfoStructure.luckyPoints = static_cast<std::uint64_t>(query.getColumn("LuckyPoints").getInt());
						playerInfoStructure.playerGrade = static_cast<std::uint64_t>(query.getColumn("Grade").getInt());
						playerInfoStructure.experience = static_cast<std::uint32_t>(query.getColumn("Experience").getInt());
						playerInfoStructure.microPoints = static_cast<std::uint64_t>(query.getColumn("MicroPoints").getInt64());
						playerInfoStructure.rockTotens = static_cast<std::uint64_t>(query.getColumn("RockTotens").getInt64());
						playerInfoStructure.inventorySpace = static_cast<std::uint32_t>(query.getColumn("MaxInventory").getInt());
						playerInfoStructure.isTutorialDone = static_cast<std::uint32_t>(query.getColumn("HasFinishedTutorial").getInt());
						playerInfoStructure.maxBattery = static_cast<std::uint32_t>(query.getColumn("MaxBattery").getInt());
						playerInfoStructure.singleWaveAttempts = static_cast<std::uint32_t>(query.getColumn("SingleWaveAttempts").getInt());
						playerInfoStructure.highestSinglewaveStage = static_cast<std::uint32_t>(query.getColumn("SingleWaveAttempts").getInt());
						playerInfoStructure.highestSingleWaveScore = static_cast<std::uint32_t>(query.getColumn("HighestSinglewaveScore").getInt());
						playerInfoStructure.clanContribution = static_cast<std::uint64_t>(query.getColumn("ClanContribution").getInt64());
						playerInfoStructure.clanLogoFrontId = static_cast<std::uint64_t>(query.getColumn("ClanFrontIcon").getInt());
						playerInfoStructure.clanLogoBackId = static_cast<std::uint64_t>(query.getColumn("ClanBackIcon").getInt());
						playerInfoStructure.clanWins = static_cast<std::uint64_t>(query.getColumn("ClanWins").getInt());
						playerInfoStructure.clanLosses = static_cast<std::uint64_t>(query.getColumn("ClanLoses").getInt());
						playerInfoStructure.clanDraws = static_cast<std::uint64_t>(query.getColumn("ClanDraws").getInt());
						playerInfoStructure.clanKills = static_cast<std::uint32_t>(query.getColumn("ClanKills").getInt());
						playerInfoStructure.clanDeaths = static_cast<std::uint32_t>(query.getColumn("ClanDeaths").getInt());
						playerInfoStructure.clanAssists = static_cast<std::uint32_t>(query.getColumn("ClanAssists").getInt());
						playerInfoStructure.infected = static_cast<std::uint32_t>(query.getColumn("InfectedKills").getInt());
						playerInfoStructure.setZombieKills(query.getColumn("ZombieKills").getInt());

						// All users have all characters by default
						std::vector<Common::Enums::Characters> boughtCharacterTypes{};
						for (std::size_t currentCharacter = 0; currentCharacter <= static_cast<std::size_t>(Common::Enums::Characters::Sophitia); ++currentCharacter)
						{
							boughtCharacterTypes.push_back(static_cast<Common::Enums::Characters>(currentCharacter));
						}
						playerInfoStructure.setBoughtCharacters(boughtCharacterTypes);
					}
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::getPlayerInfo] SQLite exception: " << e.what() << '\n';
				}

				return playerInfoStructure;
			}

			Main::Structures::MuteInfo isMuted(std::uint32_t playerID)
			{
				try
				{
					Main::Structures::MuteInfo muteInfo;

					SQLite::Statement query(db,
						"SELECT Users.* FROM Users WHERE AccountID = ?");

					query.bind(1, playerID);

					if (query.executeStep())
					{
						const std::string mutedUntil = query.getColumn("MutedUntil").getString(); // suspendedUntil uses UTC!
						auto const time = std::chrono::utc_clock::now();
						const std::string current_time = std::format("{:%Y-%m-%d %X}", time);
						muteInfo.isMuted = mutedUntil > current_time;
						muteInfo.reason = query.getColumn("MuteReason").getString();
						muteInfo.mutedBy = query.getColumn("MutedBy").getString();
						muteInfo.mutedUntil = mutedUntil;
					}
					else
					{
						std::cerr << "[Main::Database::isMuted] Error executing query!\n";
					}
					return muteInfo;
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::isMuted] SQLite exception: " << e.what() << '\n';
				}
			}

			bool unbanPlayer(const std::string& nickname)
			{
				try
				{
					std::string queryStr = "UPDATE Users SET SuspendedUntil = 0 WHERE Nickname = ?";
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, queryStr);
					query.bind(1, nickname);
					
					if (!query.exec())
					{
						std::cerr << "[Main::Database::UnbanPlayer] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::UnbanPlayer] Error message: " << query.getErrorMsg() << '\n';
						return false;
					}

					transaction.commit();
					return true;
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::UnbanPlayer] SQLite exception: " << e.what() << '\n';
					return false;
				}
			}

			bool addPlayer(const std::string& username, const std::string& password, const std::string& nickname)
			{
				try
				{
					std::cout << "Username: " << username << ", password: " << password << ", nickname: " << nickname << '\n';
					std::string queryStr = "INSERT INTO Users (Username, Password, Nickname) VALUES (?, ?, ?);";

					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, queryStr);
					query.bind(1, username);
					query.bind(2, password);
					query.bind(3, nickname);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::addPlayer] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::addPlayer] Error message: " << query.getErrorMsg() << '\n';
						return false;
					}
					transaction.commit();

					return true;
				}
				catch (const std::exception& e)
				{
					std::cerr << "Exception: " << e.what() << '\n';
					return false;
				}
			}


			auto getPlayerItems(std::uint32_t playerID) const
				-> std::pair<std::vector<Item>, std::unordered_map<std::uint16_t, std::vector<EquippedItem>>>
			{
				std::vector<Item> nonEquippedItems;
				std::unordered_map<std::uint16_t, std::vector<EquippedItem>> equippedItemsPerCharacter;

				std::uint64_t itemNum = 0;
				try
				{
					SQLite::Statement allItemsQuery(db, "SELECT rowid, * FROM UserItems WHERE AccountID = :accountID");
					allItemsQuery.bind(":accountID", playerID);

					Main::Structures::Item item;
					while (allItemsQuery.executeStep())
					{
						item.id = static_cast<std::uint32_t>(allItemsQuery.getColumn("ItemID").getInt());

						item.serialInfo.itemOrigin = static_cast<std::uint64_t>(allItemsQuery.getColumn("ItemOrigin").getInt64());
						item.serialInfo.m_serverId = static_cast<std::uint64_t>(allItemsQuery.getColumn("acquisitionServerId").getInt64());
						item.serialInfo.itemCreationDate = static_cast<__time32_t>(allItemsQuery.getColumn("creationDate").getInt64());
						std::uint32_t rowId = allItemsQuery.getColumn("rowid");

						item.serialInfo.itemNumber = ++itemNum;
						SQLite::Statement updateItemNumberQuery(db, "UPDATE UserItems SET ItemNumber = :itemNumber WHERE rowid = :rowid");
						updateItemNumberQuery.bind(":itemNumber", static_cast<std::int64_t>(item.serialInfo.itemNumber));
				        updateItemNumberQuery.bind(":rowid" ,rowId);
						if (!updateItemNumberQuery.exec())
						{
							std::cerr << "ItemNumberUpdate error in getPlayerItems(): " << updateItemNumberQuery.getExtendedErrorCode() << '\n';
							std::cerr << "Error message: " << updateItemNumberQuery.getErrorMsg() << '\n';
						}

						const std::uint64_t itemDuration_s = static_cast<std::uint64_t>(allItemsQuery.getColumn("ItemDuration").getInt64());
						if (itemDuration_s <= 2)
						{
							item.expirationDate = static_cast<__time32_t>(itemDuration_s);
						}
						else
						{
							const time_t newExpDate = static_cast<time_t>(item.serialInfo.itemCreationDate) + itemDuration_s;
							if (std::time(nullptr) >= newExpDate)
							{
								// TODO: delete the item from the database + test
							}
							item.expirationDate = static_cast<__time32_t>(newExpDate);
						}
						item.durability = static_cast<std::uint16_t>(allItemsQuery.getColumn("durability").getInt());
						item.energy = static_cast<std::uint16_t>(allItemsQuery.getColumn("energy").getInt());
						if (allItemsQuery.getColumn("IsEquipped").getInt() == 1)
						{
							Main::Structures::EquippedItem equippedItem{ item };
							equippedItemsPerCharacter[allItemsQuery.getColumn("CharacterID").getInt()].push_back(equippedItem);
						}
						else
						{
							nonEquippedItems.push_back(item);
						}
					}

				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::getPlayerInfo] SQLite exception: " << e.what() << '\n';
				}

				return std::pair{ nonEquippedItems, equippedItemsPerCharacter };
			}

			void addPlayerItems(std::uint32_t accountID, const std::vector<Item>& items, std::uint32_t latestCharacterSelected = 0)
			{
				for (const auto& currentItem : items)
				{
					addPlayerItem(currentItem, accountID, latestCharacterSelected);
				}
			}

			void replaceItem(std::uint32_t accountID, std::uint32_t itemNum, const Main::Structures::ItemSerialInfo& newSerialInfo, 
				std::uint64_t newExpiration)
			{
				try
				{
					std::string updateLevelQuery = "UPDATE UserItems SET ItemDuration = ?, ItemOrigin = ?, acquisitionServerId = ?, creationDate = ? WHERE AccountID = ? AND ItemNumber = ?";
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, updateLevelQuery);
					query.bind(1, static_cast<std::int64_t>(newExpiration));
					query.bind(2, static_cast<int64_t>(newSerialInfo.itemOrigin));
					query.bind(3, static_cast<std::int64_t>(newSerialInfo.m_serverId));
					query.bind(4, static_cast<std::int64_t>(newSerialInfo.itemCreationDate));
					query.bind(5, static_cast<std::int64_t>(accountID));
					query.bind(6, static_cast<std::int32_t>(itemNum));

					if (!query.exec())
					{
						std::cerr << "[Main::Database::replaceItem] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::replaceItem] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::replaceItem] SQLite exception: " << e.what() << '\n';
				}
			}
			void addPlayerItem(const Item& item, std::uint32_t accountID, std::uint32_t latestCharacterSelected = 0)
			{
				try
				{
					SQLite::Statement query(db,
						"INSERT INTO UserItems (AccountID, IsEquipped, CharacterID, ItemID, ItemDuration, ItemNumber, ItemOrigin, acquisitionServerId, creationDate, durability, energy, isSealed, sealLevel, expEnhancement, mpEnhancement, unknown) "
						"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
					SQLite::Transaction transaction(db);

					query.bind(1, accountID);
					query.bind(2, latestCharacterSelected == 0 ? 0 : 1);
					query.bind(3, latestCharacterSelected);
					query.bind(4, item.id);
					query.bind(5, static_cast<std::int32_t>(item.expirationDate));
					query.bind(6, static_cast<std::int32_t>(item.serialInfo.itemNumber));
					query.bind(7, static_cast<std::int16_t>(item.serialInfo.itemOrigin));
					query.bind(8, static_cast<std::int16_t>(item.serialInfo.m_serverId));
					query.bind(9, static_cast<std::int32_t>(item.serialInfo.itemCreationDate));
					query.bind(10, static_cast<std::uint32_t>(item.durability));
					query.bind(11, static_cast<std::uint32_t>(item.energy));
					query.bind(12, 0);
					query.bind(13, 0);
					query.bind(14, 0);
					query.bind(15, 0);
					query.bind(16, 0);
					query.exec();
					transaction.commit();
				}
				catch (std::exception& e)
				{
					std::cerr << "Main::Database::DatabaseManager::addPlayerItem exception: " << e.what() << '\n';
				}
			}

			void removePlayerItem(std::uint32_t accountId, std::uint32_t itemNumber)
			{
				try
				{
					const std::string deleteItem = "DELETE FROM UserItems WHERE AccountID = ? AND ItemNumber = ?";
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, deleteItem);

					query.bind(1, accountId);
					query.bind(2, itemNumber);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::removePlayerItem] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::removePlayerItem] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::removePlayerItem] SQLite exception: " << e.what() << '\n';
				}
			}


			void updatePlayerLevel(std::uint32_t accountID, std::uint16_t level)
			{
				try
				{
					std::string updateLevelQuery = "UPDATE Users SET Level = ? WHERE AccountID = ?";
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, updateLevelQuery);
					query.bind(1, level);
					query.bind(2, accountID);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::updatePlayerLevel] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::updatePlayerLevel] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::updatePlayerLevel] SQLite exception: " << e.what() << '\n';
				}
			}

			void updatePlayerName(std::uint32_t accountID, const char* name)
			{
				try
				{
					std::string updateNameQuery = "UPDATE Users SET Nickname = ? WHERE AccountID = ?";
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, updateNameQuery);
					query.bind(1, name);
					query.bind(2, accountID);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::updatePlayerName] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::updatePlayerName] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::updatePlayerName] SQLite exception: " << e.what() << '\n';
				}
			}

			void updateSuspension(std::uint32_t accountID, const std::string& until, const std::string& reason)
			{
				try
				{
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, "UPDATE Users SET SuspendedUntil = ?, SuspensionReason = ? WHERE AccountID = ?");
					query.bind(1, until);
					query.bind(2, reason);
					query.bind(3, accountID);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::updateSuspension] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::updateSuspension] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::updateSuspension] SQLite exception: " << e.what() << '\n';
				}
			}

			void updateOfflineStatus(std::uint32_t accountId)
			{
				try
				{
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, "UPDATE Users SET IsOnline = 0 WHERE AccountID = ?");
					query.bind(1, accountId);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::updateOfflineStatus] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::updateOfflineStatus] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::updateOfflineStatus] SQLite exception: " << e.what() << '\n';
				}
			}

			void updateOnlineStatus(std::uint32_t accountId)
			{
				try
				{
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, "UPDATE Users SET IsOnline = 1 WHERE AccountID = ?");
					query.bind(1, accountId);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::updateOnlineStatus] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::updateOnlineStatus] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::updateOnlineStatus] SQLite exception: " << e.what() << '\n';
				}
			}

			void updateBattery(std::uint32_t accountId, std::uint32_t newBattery)
			{
				try
				{
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, "UPDATE Users SET Battery = ? WHERE AccountID = ?");
					query.bind(1, newBattery);
					query.bind(2, accountId);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::updateBattery] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::updateBattery] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::updateBattery] SQLite exception: " << e.what() << '\n';
				}
			}

			void updatePlayerStats(std::uint32_t accountId, const Main::Structures::AccountInfo& updatedAccountInfo)
			{
				try
				{
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, "UPDATE Users SET MeleeKills = ?, RifleKills = ?, ShotgunKills = ?, SniperKills = ?, GatlingKills = ?, " 
						 "BazookaKills = ? , GrenadeKills = ? , HighestKillstreak = ? , Kills = ? , Deaths = ? , Headshots = ? , Assists = ?, "
						" Experience = ?, MicroPoints = ?, Wins = ?, Loses = ?, Draws = ?, Level = ? WHERE AccountID = ?");

					query.bind(1, updatedAccountInfo.meleeKills);
					query.bind(2, updatedAccountInfo.rifleKills);
					query.bind(3, updatedAccountInfo.shotgunKills);
					query.bind(4, updatedAccountInfo.sniperKills);
					query.bind(5, updatedAccountInfo.microgunKills);
					query.bind(6, updatedAccountInfo.bazookaKills);
					query.bind(7, updatedAccountInfo.grenadeKills);
					query.bind(8, static_cast<std::uint32_t>(updatedAccountInfo.killstreak));
					query.bind(9, updatedAccountInfo.totalKills);
					query.bind(10, updatedAccountInfo.deaths);
					query.bind(11, static_cast<std::uint32_t>(updatedAccountInfo.headshots));
					query.bind(13, updatedAccountInfo.experience);
					query.bind(14, static_cast<std::uint32_t>(updatedAccountInfo.microPoints));
					query.bind(15, updatedAccountInfo.wins);
					query.bind(16, updatedAccountInfo.losses);
					query.bind(17, updatedAccountInfo.draws);
					query.bind(18, static_cast<std::uint32_t>(updatedAccountInfo.playerLevel - 1));
					query.bind(19, accountId);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::updatePlayerStats] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::updatePlayerStats] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::updatePlayerStats] SQLite exception: " << e.what() << '\n';
				}
			}

			void updateMute(std::uint32_t accountID, const std::string& until, const std::string& reason, const std::string& mutedBy)
			{
				try
				{
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, "UPDATE Users SET MutedUntil = ?, MuteReason = ?, MutedBy = ? WHERE AccountID = ?");
					query.bind(1, until);
					query.bind(2, reason);
					query.bind(3, mutedBy);
					query.bind(4, accountID);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::updateMute] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::updateMute] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::updateMute] SQLite exception: " << e.what() << '\n';
				}
			}

			void unmuteAccount(std::uint32_t accountID)
			{
				try
				{
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, "UPDATE Users SET MutedUntil = 0 WHERE AccountID = ?");
					query.bind(1, accountID);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::unmuteAccount] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::unmuteAccount] Error message: " << query.getErrorMsg() << '\n';
						return;
					}
					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::unmuteAccount] SQLite exception: " << e.what() << '\n';
				}
			}

			void switchItemEquip(std::uint32_t accountID, std::uint64_t itemNumber, std::uint32_t characterId)
			{
				try
				{
					SQLite::Transaction transaction(db);

					SQLite::Statement query(db, "UPDATE UserItems SET IsEquipped = CASE WHEN IsEquipped = 0 THEN 1 ELSE 0 END, CharacterID = ? WHERE AccountID = ? AND ItemNumber = ?");

					query.bind(1, characterId);
					query.bind(2, accountID);
					query.bind(3, static_cast<std::int64_t>(itemNumber));

					if (!query.exec())
					{
						std::cerr << "[Main::Database::switchItemEquip] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::switchItemEquip] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::switchItemEquip] SQLite exception: " << e.what() << '\n';
				}
			}


			void unequipItem(std::uint32_t accountID, std::uint64_t unequipItemNumber)
			{
				try
				{
					SQLite::Transaction transaction(db);

					SQLite::Statement query(db, "UPDATE UserItems SET IsEquipped = 0, CharacterID = 0 WHERE AccountID = ? AND ItemNumber = ?");

					query.bind(1, accountID);
					query.bind(2, static_cast<std::int64_t>(unequipItemNumber));

					if (!query.exec())
					{
						std::cerr << "[Main::Database::unequipItems] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::unequipItems] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::unequipItems] SQLite exception: " << e.what() << '\n';
				}
			}

			void equipItem(std::uint32_t accountID, std::uint64_t equipItemNumber, std::uint16_t characterId)
			{
				try
				{
					SQLite::Transaction transaction(db);

					// Update equipped items
					SQLite::Statement query(db, "UPDATE UserItems SET IsEquipped = 1, CharacterID = ? WHERE AccountID = ? AND ItemNumber = ?");
					query.bind(1, static_cast<std::int32_t>(characterId));
					query.bind(2, accountID);
					query.bind(3, static_cast<std::int64_t>(equipItemNumber));

					if (!query.exec())
					{
						std::cerr << "[Main::Database::equipItem] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::equipItem] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::equipItem] SQLite exception: " << e.what() << '\n';
				}
			}

			void swapItems(std::uint32_t accountID, std::uint64_t toUnequipItemNumber, std::uint64_t toEquipItemNumber, std::uint16_t characterId)
			{
				try
				{
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, "UPDATE UserItems SET IsEquipped = 1, CharacterID = ? WHERE AccountID = ? AND ItemNumber = ?");
					query.bind(1, static_cast<std::int32_t>(characterId));
					query.bind(2, accountID);
					query.bind(3, static_cast<std::int64_t>(toEquipItemNumber));

					SQLite::Statement query2(db, "UPDATE UserItems SET IsEquipped = 0, CharacterID = 0 WHERE AccountID = ? AND ItemNumber = ?");
					query2.bind(1, accountID);
					query2.bind(2, static_cast<std::int64_t>(toUnequipItemNumber));

					if (!query.exec())
					{
						std::cerr << "[Main::Database::swapItems] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::swapItems] Error message: " << query.getErrorMsg() << '\n';
						return;
					}
					if (!query2.exec())
					{
						std::cerr << "[Main::Database::swapItems] Error executing query2: " << query2.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::swapItems] Error message: " << query2.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::swapItems] SQLite exception: " << e.what() << '\n';
				}
			}

			void addFriend(std::uint32_t accountID, std::uint32_t targetAccountId)
			{
				try
				{
					const std::string insertFriendQuery = "INSERT INTO Friendlist (AccountID, TargetAccountID) VALUES (?, ?)";
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, insertFriendQuery);

					query.bind(1, accountID);
					query.bind(2, targetAccountId);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::addFriend] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::addFriend] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::addFriend] SQLite exception: " << e.what() << '\n';
				}
			}

			bool resetKillDeath(std::uint32_t accountID)
			{
				try
				{
					const std::string resetKills = "UPDATE Users SET Kills = 0 WHERE AccountID = ?";
					const std::string resetDeaths = "UPDATE Users SET Deaths = 0 WHERE AccountID = ?";

					SQLite::Transaction transaction(db);
					SQLite::Statement resetKillsQuery(db, resetKills);
					SQLite::Statement resetDeathsQuery(db, resetDeaths);
					resetKillsQuery.bind(1, accountID);
					resetDeathsQuery.bind(1, accountID);

					const int killsResult = resetKillsQuery.exec();
					const int deathsResult = resetDeathsQuery.exec();
					if (killsResult <= 0 || deathsResult <= 0)
					{
						std::cerr << "[Main::Database::resetKillDeath] Error executing query: no rows affected\n";
						return false;
					}

					transaction.commit();
					return true;
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::resetKillDeath] SQLite exception: " << e.what() << '\n';
					return false;
				}
			}


			bool resetRecord(std::uint32_t accountID)
			{
				try
				{
					const std::string resetWins = "UPDATE Users SET Wins = 0 WHERE AccountID = ?";
					const std::string resetLosses = "UPDATE Users SET Loses = 0 WHERE AccountID = ?";
					const std::string resetDraws = "UPDATE Users SET Draws = 0 WHERE AccountID = ?";

					SQLite::Transaction transaction(db);
					SQLite::Statement resetWinsQuery(db, resetWins);
					SQLite::Statement resetLosesQuery(db, resetLosses);
					SQLite::Statement resetDrawsQuery(db, resetDraws);

					resetWinsQuery.bind(1, accountID);
					resetLosesQuery.bind(1, accountID);
					resetDrawsQuery.bind(1, accountID);

					const int winsResult = resetWinsQuery.exec();
					const int losesResults = resetLosesQuery.exec();
					const int drawsResults = resetDrawsQuery.exec();
					if (winsResult <= 0 || losesResults <= 0 || drawsResults <= 0)
					{
						std::cerr << "[Main::Database::resetRecord] Error executing query: no rows affected\n";
						return false;
					}

					transaction.commit();
					return true;
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::resetRecord] SQLite exception: " << e.what() << '\n';
					return false;
				}
			}

			bool batteryRecharge(std::uint32_t accountID, std::uint32_t quantity)
			{
				try
				{
					const std::string getBattery = "SELECT Battery FROM Users WHERE AccountID = ?";
					const std::string updateBattery = "UPDATE Users SET Battery = ? WHERE AccountID = ?";

					SQLite::Transaction transaction(db);
					SQLite::Statement getBatteryQuery(db, getBattery);
					getBatteryQuery.bind(1, accountID);
					if (!getBatteryQuery.executeStep())
					{
						std::cerr << "[Main::Database::batteryRecharge] Error: Account not found\n";
						return false;
					}

					std::uint32_t oldBattery = getBatteryQuery.getColumn(0).getUInt();
					std::uint32_t newBattery = oldBattery + quantity;

					SQLite::Statement updateBatteryQuery(db, updateBattery);
					updateBatteryQuery.bind(1, newBattery);
					updateBatteryQuery.bind(2, accountID);

					if (updateBatteryQuery.exec() <= 0)
					{
						std::cerr << "[Main::Database::batteryRecharge] Error executing update: no rows affected\n";
						return false;
					}
					transaction.commit();
					return true;
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::batteryRecharge] SQLite exception: " << e.what() << '\n';
					return false;
				}
			}


			bool batteryExpansion(std::uint32_t accountID)
			{
				try
				{
					const std::string getMaxBattery = "SELECT MaxBattery FROM Users WHERE AccountID = ?";
					const std::string updateMaxBattery = "UPDATE Users SET MaxBattery = ? WHERE AccountID = ?";

					SQLite::Transaction transaction(db);
					SQLite::Statement getMaxBatteryQuery(db, getMaxBattery);
					getMaxBatteryQuery.bind(1, accountID);
					if (!getMaxBatteryQuery.executeStep())
					{
						std::cerr << "[Main::Database::batteryExpansion] Error: Account not found\n";
						return false;
					}

					std::uint32_t currentMaxBattery = getMaxBatteryQuery.getColumn(0).getUInt();
					if (currentMaxBattery >= 4000)
					{
						std::cout << "[Main::Database::batteryExpansion] MaxBattery is already at or above 4000, no expansion needed.\n";
						return false;
					}

					const std::uint32_t newMaxBattery = currentMaxBattery + 1000;
					SQLite::Statement updateMaxBatteryQuery(db, updateMaxBattery);
					updateMaxBatteryQuery.bind(1, newMaxBattery);
					updateMaxBatteryQuery.bind(2, accountID);

					if (updateMaxBatteryQuery.exec() <= 0)
					{
						std::cerr << "[Main::Database::batteryExpansion] Error executing update: no rows affected\n";
						return false;
					}
					transaction.commit();
					return true;
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::batteryExpansion] SQLite exception: " << e.what() << '\n';
					return false;
				}
			}


			void removeFriend(std::uint32_t accountID, std::uint32_t targetAccountId)
			{
				try
				{
					const std::string deleteFriend = "DELETE FROM Friendlist WHERE AccountID = ? AND TargetAccountID = ?";

					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, deleteFriend);
					query.bind(1, accountID);
					query.bind(2, targetAccountId);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::removeFriend] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::removeFriend] Error message: " << query.getErrorMsg() << '\n';
						return;
					}
					query.reset();
					query.bind(1, targetAccountId);
					query.bind(2, accountID);
					if (!query.exec())
					{
						std::cerr << "[Main::Database::removeFriend] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::removeFriend] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::removeFriend] SQLite exception: " << e.what() << '\n';
				}
			}

			std::vector<Main::Structures::Friend> loadFriends(std::uint32_t accountID) const
			{
				std::vector<Main::Structures::Friend> friendList;

				try
				{
					const std::string selectFriendsQuery = "SELECT Friendlist.*, Users.Nickname AS TargetNickname "
						"FROM Friendlist "
						"INNER JOIN Users ON Friendlist.TargetAccountID = Users.AccountID "
						"WHERE Friendlist.AccountID = ?";
					SQLite::Statement query(db, selectFriendsQuery);
					query.bind(1, accountID);

					Main::Structures::Friend ffriend;
					while (query.executeStep())
					{
						std::memcpy(ffriend.targetNickname, query.getColumn("TargetNickname").getString().c_str(), 16);
						ffriend.targetAccountId = query.getColumn("TargetAccountID").getInt();

						friendList.push_back(ffriend);
					}
				}
				catch (const std::exception& e)
				{
				}

				return friendList;
			}

			std::vector<Main::Structures::BlockedPlayer> loadBlockedPlayers(std::uint32_t accountID) const
			{
				std::vector<Main::Structures::BlockedPlayer> blockedList;

				try
				{
					const std::string selectBlockedPlayersQuery = "SELECT BlockedPlayers.*, Users.Nickname AS TargetNickname "
						"FROM BlockedPlayers "
						"INNER JOIN Users ON BlockedPlayers.TargetAccountID = Users.AccountID "
						"WHERE BlockedPlayers.AccountID = ?";
					SQLite::Statement query(db, selectBlockedPlayersQuery);
					query.bind(1, accountID);

					Main::Structures::BlockedPlayer blockedPlayer;
					while (query.executeStep())
					{
						std::memcpy(blockedPlayer.targetNickname, query.getColumn("TargetNickname").getString().c_str(), 16);
						blockedPlayer.targetAccountId = query.getColumn("TargetAccountID").getInt();

						blockedList.push_back(blockedPlayer);
					}
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::loadBlockedPlayers] SQLite exception: " << e.what() << '\n';
				}

				return blockedList;
			}

			void blockPlayer(std::uint32_t accountID, std::uint32_t targetAccountId)
			{
				try
				{
					const std::string blockPlayerQuery = "INSERT INTO BlockedPlayers (AccountID, TargetAccountID) VALUES (?, ?)";
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, blockPlayerQuery);

					query.bind(1, accountID);
					query.bind(2, targetAccountId);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::blockPlayer] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::blockPlayer] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::blockPlayer] SQLite exception: " << e.what() << '\n';
				}
			}

			std::uint32_t blockPlayerByNickname(std::uint32_t accountID, const std::string& targetNickname)
			{
				try
				{
					const std::string findTargetAccountIdQuery = "SELECT AccountID FROM Users WHERE Nickname = ?";
					SQLite::Statement findQuery(db, findTargetAccountIdQuery);
					findQuery.bind(1, targetNickname);
					if (!findQuery.executeStep())
					{
						std::cerr << "[Main::Database::blockPlayer] Nickname not found: " << targetNickname << '\n';
						return -1;
					}

					auto targetAccountId = findQuery.getColumn(0).getUInt();
					const std::string blockPlayerQuery = "INSERT INTO BlockedPlayers (AccountID, TargetAccountID) VALUES (?, ?)";
					SQLite::Transaction transaction(db);
					SQLite::Statement blockQuery(db, blockPlayerQuery);
					blockQuery.bind(1, accountID);
					blockQuery.bind(2, targetAccountId);

					if (!blockQuery.exec())
					{
						std::cerr << "[Main::Database::blockPlayer] Error executing block query: " << blockQuery.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::blockPlayer] Error message: " << blockQuery.getErrorMsg() << '\n';
						return -1;
					}
					transaction.commit();
					return targetAccountId;
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::blockPlayer] SQLite exception: " << e.what() << '\n';
					return -1;
				}
			}


			void unblockPlayer(std::uint32_t accountID, std::uint32_t targetAccountId)
			{
				try
				{
					const std::string unblockPlayerQuery = "DELETE FROM BlockedPlayers WHERE AccountID = ? AND TargetAccountID = ?";

					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, unblockPlayerQuery);
					query.bind(1, accountID);
					query.bind(2, targetAccountId);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::unblockPlayer] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::unblockPlayer] Error message: " << query.getErrorMsg() << '\n';
						return;
					}
					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::removeFriend] SQLite exception: " << e.what() << '\n';
				}
			}

			Main::Enums::AddFriendServerExtra addPendingFriendRequest(std::uint32_t aid, const char* targetName)
			{
				try
				{
					std::uint32_t targetAid = 0;

					// 1. Find the player.
					SQLite::Statement findPlayerByAccountId(db, "SELECT * FROM Users WHERE Nickname = ?");
					findPlayerByAccountId.bind(1, targetName);
					if (findPlayerByAccountId.executeStep())
					{
						targetAid = static_cast<std::uint32_t>(findPlayerByAccountId.getColumn("AccountID").getInt());
					}
					else
					{
						std::cerr << "[Main::Database::addPendingFriendRequestFor] Error executing query: " << findPlayerByAccountId.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::addPendingFriendRequestFor] Error message: " << findPlayerByAccountId.getErrorMsg() << '\n';
						return Main::Enums::AddFriendServerExtra::TARGET_NOT_FOUND;
					}
					
					// 2. Check if the target player has less than 30 friends.
					SQLite::Statement findTotalFriends(db, "SELECT * FROM Friendlist WHERE AccountID = ?");
					findTotalFriends.bind(1, targetAid);
					std::size_t totalCount = 0;
					if (!findTotalFriends.executeStep())
					{
						std::cerr << "[Main::Database::addPendingFriendRequestFor::findTotalFriends] Error executing query: " << findTotalFriends.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::addPendingFriendRequestFor::findTotalFriends] Error message: " << findTotalFriends.getErrorMsg() << '\n';
						totalCount = 0;
					}
					else 
					{
						totalCount = 1;
						while (findTotalFriends.executeStep())
						{
							totalCount++;
						}
						if (totalCount > 30)
						{
							return Main::Enums::AddFriendServerExtra::TARGET_OR_SENDER_FRIEND_LIST_FULL;
						}
					}

					// 3. Check if the target player blocked us. 
					SQLite::Statement hasBlockedUs(db, "SELECT * FROM BlockedPlayers WHERE AccountID = ? AND TargetAccountID = ?");
					hasBlockedUs.bind(1, targetAid);
					hasBlockedUs.bind(2, aid);
					if (hasBlockedUs.hasRow())
					{
						return Main::Enums::AddFriendServerExtra::RECEIVER_BLOCKED_SENDER;
					}

					SQLite::Transaction transaction(db);

					// 4. Add pending friend request to target player
					SQLite::Statement insertPendingRequest(db, "INSERT INTO PendingFriendRequests (AccountID, TargetAccountID) VALUES(?, ?)");
					insertPendingRequest.bind(1, targetAid);
					insertPendingRequest.bind(2, aid);
					if (!insertPendingRequest.exec())
					{
						std::cerr << "[Main::Database::addPendingFriendRequestFor::insertPendingRequest] Error executing query: " << findTotalFriends.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::addPendingFriendRequestFor::insertPendingRequest] Error message: " << findTotalFriends.getErrorMsg() << '\n';
						return Main::Enums::AddFriendServerExtra::DB_ERROR;
					}
					transaction.commit();
					return Main::Enums::AddFriendServerExtra::REQUEST_SENT;
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::addPendingFriendRequestFor] SQLite exception: " << e.what() << '\n';
				}
			}

			std::vector<Main::Structures::Friend> loadPendingFriendRequests(std::uint32_t accountID)
			{
				std::vector<Main::Structures::Friend> pendingFriendRequests;

				try
				{
					const std::string pendingFriendRequestsQuery = "SELECT PendingFriendRequests.*, Users.Nickname AS TargetNickname "
						"FROM PendingFriendRequests "
						"INNER JOIN Users ON PendingFriendRequests.TargetAccountID = Users.AccountID "
						"WHERE PendingFriendRequests.AccountID = ?";
					SQLite::Statement query(db, pendingFriendRequestsQuery);
					query.bind(1, accountID);

					Main::Structures::Friend ffriend;
					while (query.executeStep())
					{
						std::memcpy(ffriend.targetNickname, query.getColumn("TargetNickname").getString().c_str(), 16);
						ffriend.targetAccountId = query.getColumn("TargetAccountID").getInt();
						pendingFriendRequests.push_back(ffriend);
					}

					SQLite::Transaction transaction(db);
					SQLite::Statement removeQuery(db, "DELETE FROM PendingFriendRequests WHERE AccountID = ?");
					removeQuery.bind(1, accountID);
					if (!removeQuery.exec())
					{
					}
					transaction.commit();
				}
				catch (const std::exception& e)
				{
				}

				return pendingFriendRequests;
			}

			std::vector<Main::Structures::SingleModeEvent> getEventsModeList()
			{
				SQLite::Statement query(db, "SELECT * FROM EventsModes");

				std::vector<Main::Structures::SingleModeEvent> events;
				Main::Structures::SingleModeEvent singleEvent;

				while (query.executeStep())
				{
					singleEvent.gameMode = static_cast<Common::Enums::GameModes>(query.getColumn("GameMode").getInt());
					singleEvent.startDate = static_cast<__time32_t>(query.getColumn("StartDate").getInt64());
					singleEvent.endDate = static_cast<__time32_t>(query.getColumn("EndDate").getInt64());
					events.push_back(singleEvent);
				}

				return events;
			}

			std::vector<Main::Structures::SingleMapEvent> getEventsMapList()
			{
				SQLite::Statement query(db, "SELECT * FROM EventsMaps");

				std::vector<Main::Structures::SingleMapEvent> events;
				Main::Structures::SingleMapEvent singleEvent;

				while (query.executeStep())
				{
					singleEvent.gameMap = static_cast<Common::Enums::GameMaps>(query.getColumn("GameMap").getInt());
					singleEvent.startDate = static_cast<__time32_t>(query.getColumn("StartDate").getInt64());
					singleEvent.endDate = static_cast<__time32_t>(query.getColumn("EndDate").getInt64());
					events.push_back(singleEvent);
				}

				return events;
			}

			void storeMailbox(const Main::Structures::Mailbox& mailbox, std::uint32_t accountId, bool isSent)
			{
				try
				{
					const std::string storeMailboxQuery = "INSERT INTO Mailbox (accountId, timestamp, uniqueId, nickname, message, sent) VALUES (?, ?, ?, ?, ?, ?)";
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, storeMailboxQuery);

					query.bind(1, accountId);
					query.bind(2, mailbox.timestamp);
					//query.bind(3, mailbox.uniqueId);
					query.bind(4, mailbox.nickname);
					query.bind(5, mailbox.message);
					query.bind(6, isSent);


					if (!query.exec())
					{
						std::cerr << "[Main::Database::storeMailbox] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::storeMailbox] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::storeMailbox] SQLite exception: " << e.what() << '\n';
				}
			}

			std::uint32_t storeOfflineMailbox(const Main::Structures::Mailbox& mailbox, const char* senderNickname)
			{
				try
				{
					std::uint32_t accountId = 0; 
					const std::string retrieveAccountIdQuery = "SELECT AccountID FROM Users WHERE Nickname = ?";
					SQLite::Statement retrieveQuery(db, retrieveAccountIdQuery);
					retrieveQuery.bind(1, mailbox.nickname);

					if (retrieveQuery.executeStep())
					{
						accountId = retrieveQuery.getColumn("AccountID").getInt(); 
					}
					else
					{
						std::cerr << "[Main::Database::storeMailbox] No user found with nickname: " << mailbox.nickname << '\n';
						return 0;
					}

					const std::string storeMailboxQuery = "INSERT INTO Mailbox (accountId, timestamp, uniqueId, nickname, message, sent, isNew) VALUES (?, ?, ?, ?, ?, ?, ?)";
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, storeMailboxQuery);

					query.bind(1, accountId);
					query.bind(2, mailbox.timestamp);
					//query.bind(3, mailbox.uniqueId); 
					query.bind(4, senderNickname);
					query.bind(5, mailbox.message);
					query.bind(6, false); 
					query.bind(7, true);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::storeOfflineMailbox] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::storeOfflineMailbox] Error message: " << query.getErrorMsg() << '\n';
						return 0;
					}

					transaction.commit();
					return accountId;
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::storeOfflineMailbox] SQLite exception: " << e.what() << '\n';
				}
			}

			std::vector<Main::Structures::Mailbox> getNewMailboxes(std::uint32_t accountID)
			{
				std::vector<Main::Structures::Mailbox> mailboxes;

				try
				{
					const std::string selectMailboxQuery = "SELECT * FROM Mailbox WHERE accountId = ? AND isNew = 1";
					SQLite::Statement query(db, selectMailboxQuery);
					query.bind(1, accountID);

					Main::Structures::Mailbox mailbox;
					while (query.executeStep())
					{
						mailbox.accountId = query.getColumn("accountId").getInt();
						mailbox.timestamp = query.getColumn("timestamp").getInt();
						std::memcpy(mailbox.nickname, query.getColumn("nickname").getString().c_str(), 16);
						std::memcpy(mailbox.message, query.getColumn("message").getString().c_str(), 256);
						mailboxes.push_back(mailbox);
					}
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::getNewMailboxes] SQLite exception: " << e.what() << '\n';
				}
				return mailboxes;
			}

			void updateReadMailbox(std::uint32_t accountID, std::uint32_t timestamp)
			{
				try
				{
					const std::string updateReadMailboxQuery = "UPDATE Mailbox SET isNew = 0 WHERE accountId = ? AND timestamp = ?";
					SQLite::Statement query(db, updateReadMailboxQuery);
					query.bind(1, accountID);
					query.bind(2, timestamp);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::updateReadMailbox] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::updateReadMailbox] Error message: " << query.getErrorMsg() << '\n';
						return;
					}
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::updateReadMailbox] SQLite exception: " << e.what() << '\n';
				}
			}


			void deleteMailbox(std::uint32_t timestamp, std::uint32_t accountId, bool isSent)
			{
				try
				{
					const std::string deleteMailboxQuery = "DELETE FROM Mailbox WHERE timestamp = ? AND accountId = ? AND sent = ?";
					SQLite::Transaction transaction(db);
					SQLite::Statement query(db, deleteMailboxQuery);

					query.bind(1, timestamp);
					query.bind(2, accountId);
					query.bind(3, isSent);

					if (!query.exec())
					{
						std::cerr << "[Main::Database::deleteMailbox] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::deleteMailbox] Error message: " << query.getErrorMsg() << '\n';
						return;
					}

					transaction.commit();
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::deleteMailbox] SQLite exception: " << e.what() << '\n';
				}
			}

			std::vector<Main::Structures::Mailbox> loadMailboxes(std::uint32_t accountID, bool sent)
			{
				std::vector<Main::Structures::Mailbox> mailboxes;

				try
				{
					const std::string selectMailboxQuery = "SELECT * FROM Mailbox WHERE accountId = ? AND sent = ?";
					SQLite::Statement query(db, selectMailboxQuery);
					query.bind(1, accountID);
					query.bind(2, sent);

					Main::Structures::Mailbox mailbox;
					while (query.executeStep())
					{
						mailbox.accountId = query.getColumn("accountId").getInt();
						mailbox.timestamp = query.getColumn("timestamp").getInt();
						mailbox.hasBeenRead = true;
						std::memcpy(mailbox.nickname, query.getColumn("nickname").getString().c_str(), 16);
						std::memcpy(mailbox.message, query.getColumn("message").getString().c_str(), 256);
						mailboxes.push_back(mailbox);
					}
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::loadMailbox] SQLite exception: " << e.what() << '\n';
				}

				return mailboxes;
			}

			void insertEnergyToItem(std::uint32_t accountID, std::uint32_t itemNumber, std::uint32_t newItemEnergy, std::uint32_t newTotalEnergy)
			{
				try
				{
					const std::string setEnergyForItem = "UPDATE UserItems SET energy = ? WHERE AccountID = ? AND ItemNumber = ?";
					SQLite::Statement query(db, setEnergyForItem);
					query.bind(1, newItemEnergy);
					query.bind(2, accountID);
					query.bind(3, itemNumber);

					const std::string removeEnergyFromTotalEnergy = "UPDATE Users SET Battery = ? WHERE AccountID = ?";
					SQLite::Statement query2(db, removeEnergyFromTotalEnergy);
					query2.bind(1, newTotalEnergy);
					query2.bind(2, accountID);

					if (!query.exec() || !query2.exec())
					{
						std::cerr << "[Main::Database::insertEnergyToItem] Error executing query: " << query.getExpandedSQL() << '\n';
						std::cerr << "[Main::Database::insertEnergyToItem] Error message: " << query.getErrorMsg() << '\n';
						return;
					}
				}
				catch (const std::exception& e)
				{
					std::cerr << "[Main::Database::insertEnergyToItem] SQLite exception: " << e.what() << '\n';
				}
			}

		};
	}
}

#endif
