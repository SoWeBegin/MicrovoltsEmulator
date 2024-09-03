#ifndef PLAYER_INFO_DB_H
#define PLAYER_INFO_DB_H


#include "../Structures/AuthAccountInfo.h"
#include "../AuthEnums.h"
#include <chrono>
#include <format>
#include <string>
#include <utility>
#include "../include/Network/Packet.h"
#include "../include/Utils/Utils.h"
#include <iostream>

#define SQLITECPP_COMPILE_DLL
#include "SQLiteCpp/SQLiteCpp.h"

namespace Auth
{
	namespace Persistence
	{
		class PersistentDatabase
		{
		private:
			SQLite::Database db;

		public:
			PersistentDatabase(const std::string& path);

			void addHash(std::uint32_t accountID, std::uint32_t key);

			std::pair<Common::Network::Packet, Auth::Structures::BasicAccountInfo> getPlayerInfo(const std::string& username, const std::string& password);
		};
	}
}
#endif