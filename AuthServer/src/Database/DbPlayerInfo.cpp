
#include "../../include/Database/DbPlayerInfo.h"
#include "../../include/Structures/AuthAccountInfo.h"
#include "../../include/AuthEnums.h"
#include <chrono>
#include <format>
#include <string>
#include <utility>
#include "../include/Network/Packet.h"
#include "../include/Utils/Utils.h"
#include <iostream>

namespace Auth
{
	namespace Persistence
	{
		PersistentDatabase::PersistentDatabase(const std::string& path)
			: db{ SQLite::Database(path.c_str(), SQLite::OPEN_READWRITE) }
		{
		}
		void PersistentDatabase::addHash(std::uint32_t accountID, std::uint32_t key)
		{
			try
			{
				const std::string insertHashQuery = "UPDATE Users SET AccountKey = ? WHERE AccountID = ?";
				SQLite::Transaction transaction(db);
				SQLite::Statement query(db, insertHashQuery);
				query.bind(1, key);
				query.bind(2, accountID);

				if (!query.exec())
				{
					std::cerr << "[Auth::Database::addHash] Error executing query: " << query.getExpandedSQL() << '\n';
					std::cerr << "[Auth::Database::addHash] Error message: " << query.getErrorMsg() << '\n';
					return;
				}

				transaction.commit();
			}
			catch (const std::exception& e)
			{
				std::cerr << "[Auth::Database::addHash] SQLite exception: " << e.what() << '\n';
			}
		}

		std::pair<Common::Network::Packet, Auth::Structures::BasicAccountInfo> PersistentDatabase::getPlayerInfo(const std::string& username, const std::string& password)
		{
			Common::Network::Packet playerInfo;
			Auth::Structures::BasicAccountInfo playerInfoStructure{};

			try
			{
				SQLite::Statement query(db,
					"SELECT Users.*, (DATE(Users.SuspendedUntil) >= DATE('now')) AS IsBanned, Clans.Clanname, Clans.ClanFrontIcon, Clans.ClanBackIcon FROM Users LEFT JOIN Clans ON Users.ClanID = Clans.ClanId WHERE  Username=? AND Password=?");

				query.bind(1, username);
				query.bind(2, password);

				if (query.executeStep())
				{
					const int isBanned = query.getColumn("IsBanned").getInt();
                    if (isBanned == 0)
					{
						playerInfo.setExtra(Auth::Enums::Login::SUCCESS);
						playerInfoStructure.accountId = static_cast<std::uint32_t>(query.getColumn("AccountID").getInt());
						strncpy(playerInfoStructure.playerName, query.getColumn("Nickname").getText(), sizeof(playerInfoStructure.playerName));
						strncpy(playerInfoStructure.clanName, query.getColumn("Clanname").getText(), sizeof(playerInfoStructure.clanName));
						playerInfo.setOption(static_cast<std::uint32_t>(query.getColumn("Grade").getInt()));
						playerInfoStructure.level = static_cast<std::uint32_t>(query.getColumn("Level").getInt()) + 1;
						playerInfoStructure.exp = static_cast<std::uint32_t>(query.getColumn("Experience").getInt());
						playerInfoStructure.kills = static_cast<std::uint32_t>(query.getColumn("Kills").getInt());
						playerInfoStructure.deaths = static_cast<std::uint32_t>(query.getColumn("Deaths").getInt());
						playerInfoStructure.assists = static_cast<std::uint32_t>(query.getColumn("Assists").getInt());
						playerInfoStructure.wins = static_cast<std::uint32_t>(query.getColumn("Wins").getInt());
						playerInfoStructure.losses = static_cast<std::uint32_t>(query.getColumn("Loses").getInt());
						playerInfoStructure.draws = static_cast<std::uint32_t>(query.getColumn("Draws").getInt());
						playerInfoStructure.clanIconFrontID = static_cast<std::uint16_t>(query.getColumn("ClanFrontIcon").getInt());
						playerInfoStructure.clanIconBackID = static_cast<std::uint16_t>(query.getColumn("ClanBackIcon").getInt());
						playerInfoStructure.hashKey = Common::Utils::generateHash(playerInfoStructure.accountId);
						const bool isOnline = static_cast<bool>(query.getColumn("IsOnline").getInt()); 
						if (isOnline)
						{
							playerInfo.setExtra(Auth::Enums::Login::ACCOUNT_BUSY);
						}
					}
					else
					{
						playerInfo.setExtra(Auth::Enums::Login::SUSPENDED);
					}
				}
				else
				{
					playerInfo.setExtra(Auth::Enums::Login::INCORRECT);
				}
			}
			catch (const std::exception&)
			{
				//std::cerr << e.what();
				playerInfo.setExtra(Auth::Enums::Login::DATA_ERROR);
			}

			if (playerInfoStructure.accountId != 0)
			{
				addHash(playerInfoStructure.accountId, playerInfoStructure.hashKey);
			}
			playerInfo.setData(reinterpret_cast<std::uint8_t*>(&playerInfoStructure), sizeof(playerInfoStructure));

			return std::pair{ playerInfo, playerInfoStructure };
		}
	};
}
