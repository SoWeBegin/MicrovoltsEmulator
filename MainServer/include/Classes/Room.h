#ifndef CLASS_ROOM_HEADER
#define CLASS_ROOM_HEADER

#include <cstdint>
#include <vector>
#include <string>
#include "../Structures/Room/ClientRoomCreationInfo.h"
#include "../Structures/Room/RoomPlayerInfo.h"
#include "../Structures/Room/RoomsList.h"
#include "../Structures/Room/RoomJoin.h"
#include "../Structures/Room/RoomPlayerItems.h"
#include "../Structures/Room/RoomPlayerClan.h"
#include "../Structures/Room/RoomSettingsUpdate.h"
#include "Player.h"
#include "../Network/MainSession.h"
#include "../Structures/Room/RoomPlayerInfo.h"
#include "../../include/Structures/EndScoreboard.h"

namespace Main
{
	namespace Classes
	{
		class Room
		{
		private:
			inline static std::uint16_t idCounter{};

			std::uint32_t m_tick{};
			std::uint16_t m_number{};
			std::string m_title{};    // max = 30
			std::string m_password{}; // max = 8
			bool m_hasMatchStarted{};
			bool m_isTeamBalanceOn{}; // Could not find this in any structure client<=>server, for now keep it like a separate variable...
			std::uint8_t m_specificSetting{}; // E.g eli = > num of rounds, TDM = > num of total kills / 10
			Main::Structures::RoomSettings m_settings{};
			std::vector<std::pair<Main::Structures::RoomPlayerInfo, Main::Network::Session*>> m_players{};
			std::vector<std::pair<Main::Structures::RoomPlayerInfo, Main::Network::Session*>> m_observerPlayers{};
			std::vector<std::uint32_t> m_kickedPlayerAccountIds{};

			// Points
			std::uint32_t m_bluePoints{};
			std::uint32_t m_redPoints{};

			// Other
			bool m_isMuted = false;

		public:
			Room() = default;

			explicit Room(const std::string& title, const Main::Structures::RoomSettings& settings, const Main::Structures::RoomPlayerInfo& player, Main::Network::Session* session);

			std::string getRoomInfoAsString() const
			{
				return "(RoomInfo: m_number: " + std::to_string(m_number) + ", hasMatchStarted: " + std::to_string(m_hasMatchStarted)
					+ ", TotalPlayers: " + std::to_string(m_players.size()) + ", TotalObserverPlayers: " + std::to_string(m_observerPlayers.size()) + ")";
			}

			Main::Network::Session* getHost() { return m_players[0].second; }

			Main::Network::Session* getPlayer(const Main::Structures::UniqueId& uniqueId);

			void addObserverPlayer(Main::Network::Session* session);

			void muteRoom();

			void unmuteRoom();

			bool isMuted() const;

			void removeAllPlayers();

			std::uint32_t getBestMsIndexExceptSelf(bool checkIsInMatch, std::uint64_t selfId);

			void setTick(std::uint32_t tick);

			std::uint32_t getPlayerIdx(std::uint64_t playerIndex) const;

			void setPassword(const std::string& password);
			
			const std::string& getPassword() const;

			std::uint16_t getRoomNumber() const;

			void addPlayer(Main::Network::Session* session, std::uint32_t team);

			void updatePlayerInfo(Main::Network::Session* session);

			void addKickedPlayer(std::uint32_t accountId);

			bool removeHostFromMatch();

			bool removePlayer(Main::Network::Session* session, std::uint32_t extra);

			std::vector<Main::Structures::RoomPlayerInfo> getAllPlayers() const;

			void removeHostIfAloneAndModeDoesntAllowIt();

			Main::Structures::SingleRoom getRoomInfo() const;

			const Main::Structures::RoomSettings& getRoomSettings() const;

			Main::Structures::RoomJoin getRoomJoinInfo() const;

			std::vector<Main::Structures::RoomPlayerItems> getPlayersItems() const;

			void updatePlayerItems(Main::Network::Session* session);

			Main::Structures::UniqueId getHostUniqueId() const;

			std::vector<Main::Structures::PlayerClan> getPlayersClans() const;

			void breakroom();

			void broadcastToRoom(Common::Network::Packet& packet);

			void broadcastToRoomExceptSelf(Common::Network::Packet& packet, const Main::Structures::UniqueId& uniqueId);

			// In-room chat messages
			void broadcastToTeamExceptSelf(Common::Network::Packet& packet, const Main::Structures::UniqueId& uniqueId, bool inMatch);
			void broadcastToMatchExceptSelf(Common::Network::Packet& packet, const Main::Structures::UniqueId& uniqueId, std::uint32_t extra);
			void broadcastOutsideMatchExceptSelf(Common::Network::Packet& packet, const Main::Structures::UniqueId& selfUniqueId, std::uint32_t extra);

			bool changeHost(std::size_t newHostIdx);

			bool isHost(const Main::Structures::UniqueId& uniqueId) const;

			void setSpecificSetting(std::uint8_t setting);

			void setTime(std::uint16_t time);

			std::optional<std::uint32_t> changePlayerTeam(const Main::Structures::UniqueId& uniqueId, std::uint32_t newTeam);

			void updatePlayersTeamToTeamBased();

			void updatePlayersTeamToNonTeamBased();

			Common::Enums::Team calculateNewPlayerTeam() const;

			bool isModeTeamBased() const;

			void setPlayersPerTeam(std::uint16_t playersPerTeam);

			std::uint8_t getSpecificSetting() const;

			bool kickPlayer(const std::string& name);

			bool isRoomFullObserverExcluded() const;

			void setStateFor(const Main::Structures::UniqueId& uniqueId, const Common::Enums::PlayerState& playerState);

			void startMatch(const Main::Structures::UniqueId& uniqueId);

			void endMatch();

			bool isObserverFull() const;

			std::size_t getPlayersCount() const;

			std::uint32_t getHostLevel() const;

			bool wasPreviouslyKicked(std::uint32_t accountId) const;

			template<Main::Enums::RoomSimpleSetting T> requires (T == Main::Enums::SETTING_ITEM)
			void switchSimpleSetting()
			{
				m_settings.isItemOn = ~m_settings.isItemOn;
			}

			template<Main::Enums::RoomSimpleSetting T> requires (T == Main::Enums::SETTING_OPEN)
			void switchSimpleSetting()
			{
				m_settings.isOpen = ~m_settings.isOpen;
			}

			template<Main::Enums::RoomSimpleSetting T> requires (T == Main::Enums::SETTING_TEAMBALANCE)
			void switchSimpleSetting()
			{
				m_isTeamBalanceOn = !m_isTeamBalanceOn;
			}

			template<Main::Enums::RoomSimpleSetting T> requires (T == Main::Enums::SETTING_OBSERVER)
			void switchSimpleSetting()
			{
				m_settings.isObserverModeOn = ~m_settings.isObserverModeOn;
			}

			void updateMap(std::uint16_t newMap);

			void updateRoomSettings(const Main::Structures::RoomSettingsUpdateBase& newRoomSettings, std::uint16_t newMode);

			Main::Structures::RoomSettingsUpdateTitlePassword getRoomSettingsUpdate() const;

			void updateTitle(const std::string& newTitle);

			void updatePassword(const std::string& newPassword);

			void addPoint(std::uint32_t team);

			void sendTo(const Main::Structures::UniqueId& uniqueId, const Common::Network::Packet& packet);

			void storeEndMatchStatsFor(const Main::Structures::UniqueId& uniqueId, const Main::Structures::ScoreboardResponse& stats,
				std::uint32_t blueScore, std::uint32_t redScore, bool hasLeveledUp);

			Main::Network::Session::AccountInfo getAccountInfoFor(const Main::Structures::UniqueId& uniqueId) const;
		};
	}
}

#endif

