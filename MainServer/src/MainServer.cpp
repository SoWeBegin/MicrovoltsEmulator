#include "../include/MainServer.h"
#include "Network/Session.h"
#include "Network/Packet.h"
#include "../include/Handlers/Player/InitialPlayerInfoHandlers.h"

#include "../include/Handlers/Item/BoughtItemHandler.h"
#include "../include/Handlers/Player/characterSelectionHandler.h"
#include "../include/Handlers/LobbyListHandler.h"
#include "../include/Handlers/Player/Friends/FriendPlayersListHandler.h"
#include "../include/Handlers/Player/BlockedPlayers/BlockedPlayersListHandler.h"
#include "../include/Handlers/Item/BoughtItemHandler.h"
#include "../include/Handlers/MainLobbyChatHandler.h"
#include "../include/Handlers/Player/EquippedItemsHandler.h"
#include "../include/Handlers/Player/LobbyAccountInfoHandler.h"
#include "../include/Handlers/Player/Friends/FriendsGeneralHandler.h"
#include "../include/Handlers/Player/Friends/FriendDeletionHandler.h"
#include "../include/Handlers/Player/BlockedPlayers/BlockPlayerHandler.h"
#include "../include/Handlers/Player/BlockedPlayers/BlockRemoveHandler.h"
#include "../include/Handlers/EventsHandlers.h"
#include "../include/Handlers/Trade/TradeAckHandler.h"
#include "../include/Handlers/Trade/TradeInitializationHandler.h"
#include "../include/Handlers/Player/PlayerStateHandler.h"
#include "../include/Handlers/Player/PingHandler.h"
#include "../include/Handlers/Item/GambleItemHandler.h"
#include "../include/Handlers/Trade/TradeAddItemHandler.h"
#include "../include/Handlers/Trade/TradeRemoveItemHandler.h"
#include "../include/Handlers/Trade/TradeLockHandler.h"
#include "../include/Handlers/Trade/TradeFinalizeHandler.h"
#include "../include/Handlers/Trade/TradeCancelHandler.h"
#include "../include/Handlers/Item/DeleteItemHandler.h"
#include "../include/Handlers/Item/ItemRefundHandler.h"
#include "../include/Handlers/Player/Mailbox/MailboxHandler.h"
#include "../include/Handlers/Player/Mailbox/MailboxDisplayHandler.h"
#include "../include/Handlers/Player/Mailbox/MailboxDeleteHandler.h"
#include "../include/Handlers/Player/Mailbox/MailboxReadHandler.h"
#include "../include/Handlers/Player/Mailbox/GiftDisplayHandler.h"
#include "../include/Handlers/Player/Mailbox/GiftSendHandler.h"
#include "../include/Handlers/Item/ItemUpgradeHandler.h"
#include "../include/Handlers/CapsuleSpinHandler.h"
#include "../include/Handlers/Room/RoomCreationHandler.h"
#include "../include/Handlers/Room/RoomsListHandler.h"
#include "../include/Handlers/Room/SimpleSettingHandler.h"
#include "../include/Handlers/Room/ScoreUpdateHandler.h"
#include "../include/Handlers/Room/RoomMiscHandler.h"
#include "../include/Handlers/Room/RoomLeaveHandler.h"
#include "../include/Handlers/Room/RoomJoinHandler.h"
#include "../include/Handlers/Room/RoomStartHandler.h"
#include "../include/Handlers/Room/RoomChangeHostHandler.h"
#include "../include/Handlers/Room/MatchLeaveHandler.h"
#include "../include/Handlers/LobbyClanListHandler.h"

#include "../include/Handlers/Room/EliminationNextRoundHandler.h"


#include "../include/Network/MainSession.h"
#include "../include/ChatCommands/AllCommandsIncludes.h"


namespace Main
{
	MainServer::MainServer(ioContext& io_context, const std::string & db_path, std::uint16_t port, std::uint16_t serverId)
		: m_io_context{ io_context }
		, m_acceptor{ io_context, tcp::endpoint(tcp::v4(), port) }
		, m_serverId{ serverId }
		, m_database{ db_path }
		, m_scheduler{ 120, m_database }
	{
		initializeAllCommands();

		
		Common::Network::Session::addCallback<Main::Network::Session>(52, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handlePlayerBlock(request, session, m_sessionsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(53, Main::Handlers::handlePlayerUnblock);

		Common::Network::Session::addCallback<Main::Network::Session>(54, Main::Handlers::handleBlockedPlayerList);

		Common::Network::Session::addCallback<Main::Network::Session>(57, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleLobbyClanList(request, session, m_sessionsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(61, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleGeneralFriendRequests(request, session, m_sessionsManager, m_database); });

		Common::Network::Session::addCallback<Main::Network::Session>(62, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleFriendDeletion(request, session, m_sessionsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(63, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleFriendList(request, session, m_sessionsManager, m_database); });

		Common::Network::Session::addCallback<Main::Network::Session>(66, Main::Handlers::handleMailboxGiftSend);

		Common::Network::Session::addCallback<Main::Network::Session>(67, Main::Handlers::handleMailboxGiftDisplay);

		Common::Network::Session::addCallback<Main::Network::Session>(68, [&](const Common::Network::Packet& request, 
			Main::Network::Session& session) { Main::Handlers::handleInitialPlayerInfos(request, session, m_sessionsManager, m_database, m_timeSinceLastRestart); });

		Common::Network::Session::addCallback<Main::Network::Session>(71, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handlePing(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(74, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleCharacterSelection(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(84, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleLobbyUserList(request, session, m_sessionsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(85, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleLobbyAccountInfo(request, session, m_sessionsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(86, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleBattery(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(87, Main::Handlers::handleBoughtItem);

		Common::Network::Session::addCallback<Main::Network::Session>(89, Main::Handlers::handleItemDelete);

		Common::Network::Session::addCallback<Main::Network::Session>(88, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleEquippedItemSwitch(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(92, Main::Handlers::handleCapsuleSpin);

		Common::Network::Session::addCallback<Main::Network::Session>(100, Main::Handlers::handleItemRefund);

		Common::Network::Session::addCallback<Main::Network::Session>(101, Main::Handlers::handleItemUpgrade);

		Common::Network::Session::addCallback<Main::Network::Session>(103, Main::Handlers::handleMailboxDelete);

		Common::Network::Session::addCallback<Main::Network::Session>(104, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleMailboxCommunication(request, session, m_sessionsManager, m_database); });

		Common::Network::Session::addCallback<Main::Network::Session>(105, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleReadMailbox(request, session, m_database); });

		Common::Network::Session::addCallback<Main::Network::Session>(106, Main::Handlers::handleMailboxDisplay);

		Common::Network::Session::addCallback<Main::Network::Session>(107, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomStart(request, session, m_roomsManager, m_timeSinceLastRestart); }); 

		Common::Network::Session::addCallback<Main::Network::Session>(124, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleSimpleRoomSetting<Main::Enums::SETTING_ITEM>(request, session, m_roomsManager); }); // Item on/off

		Common::Network::Session::addCallback<Main::Network::Session>(125, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomMiscellaneous(request, session, m_roomsManager); }); // Mode, Team Balance, Weapon Restriction, map

		Common::Network::Session::addCallback<Main::Network::Session>(126, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomMiscellaneous(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(127, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleSimpleRoomSetting<Main::Enums::SETTING_OPEN>(request, session, m_roomsManager); }); // open on/off

		Common::Network::Session::addCallback<Main::Network::Session>(128, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleHostChange(request, session, m_roomsManager); }); 

		Common::Network::Session::addCallback<Main::Network::Session>(129, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomMiscellaneous(request, session, m_roomsManager); }); // Password

		Common::Network::Session::addCallback<Main::Network::Session>(130, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomMiscellaneous(request, session, m_roomsManager); }); // Title,

		Common::Network::Session::addCallback<Main::Network::Session>(131, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleSimpleRoomSetting<Main::Enums::SETTING_MAP>(request, session, m_roomsManager); }); // Map (normal click)

		Common::Network::Session::addCallback<Main::Network::Session>(132, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleSimpleRoomSetting<Main::Enums::SETTING_PLAYERS_PER_TEAM>(request, session, m_roomsManager); }); // NvsN setting

		Common::Network::Session::addCallback<Main::Network::Session>(133, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleSimpleRoomSetting<Main::Enums::SETTING_OBSERVER>(request, session, m_roomsManager); }); // ObserverMode,

		Common::Network::Session::addCallback<Main::Network::Session>(134, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleSimpleRoomSetting<Main::Enums::SETTING_SPECIFIC>(request, session, m_roomsManager); }); // total kills/rounds...

		Common::Network::Session::addCallback<Main::Network::Session>(135, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleSimpleRoomSetting<Main::Enums::SETTING_TIME>(request, session, m_roomsManager); });  // time
			
		Common::Network::Session::addCallback<Main::Network::Session>(138, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomCreation(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(140, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomJoin(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(141, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomLeave(request, session, m_sessionsManager, m_roomsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(142, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomsList(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(158, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handlePlayerState(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(159, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomMiscellaneous(request, session, m_roomsManager); }); // Team switch

		Common::Network::Session::addCallback<Main::Network::Session>(161, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleLobbyChatMessage(request, session, m_sessionsManager, m_chatCommands, m_roomsManager, m_database); });

		Common::Network::Session::addCallback<Main::Network::Session>(162, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleLobbyChatMessage(request, session, m_sessionsManager, m_chatCommands, m_roomsManager, m_database); });

		Common::Network::Session::addCallback<Main::Network::Session>(191, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleTradeAck(request, session, m_sessionsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(192, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleTradeInitialization(request, session, m_sessionsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(194, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleAddTradeItem(request, session, m_sessionsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(195, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleTradeItemRemoval(request, session, m_sessionsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(196, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleTradeLock(request, session, m_sessionsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(197, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleTradeFinalization(request, session, m_sessionsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(198, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleTradeCancellation(request, session, m_sessionsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(200, Main::Handlers::handleGambleItem);

		Common::Network::Session::addCallback<Main::Network::Session>(202, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleModeEvents(request, session, m_database); });

		Common::Network::Session::addCallback<Main::Network::Session>(232, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleMapEvents(request, session, m_database); });

		Common::Network::Session::addCallback<Main::Network::Session>(256, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleMatchLeave(request, session, m_sessionsManager, m_roomsManager); });
		
		Common::Network::Session::addCallback<Main::Network::Session>(259, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleEliminationNextRound(request, session, m_roomsManager); });

		// CTB respawn 
		Common::Network::Session::addCallback<Main::Network::Session>(160, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::unknown(request, session, m_roomsManager); });

		// Bomb Battle for host
		Common::Network::Session::addCallback<Main::Network::Session>(163, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::unknown(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(108, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleEliminationNextRound2(request, session, m_roomsManager); });
		
		Common::Network::Session::addCallback<Main::Network::Session>(254, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleMatchEnding(request, session, m_roomsManager); });
			
	}

	void MainServer::asyncAccept()
	{
		m_socket.emplace(m_io_context);
		m_acceptor.async_accept(*m_socket, [&](asio::error_code error)
			{
				const auto durationSinceEpoch = std::chrono::system_clock::now().time_since_epoch();
				m_timeSinceLastRestart = static_cast<std::uint64_t>(duration_cast<std::chrono::milliseconds>(durationSinceEpoch).count());

				m_sessionsManager.setRoomsManager(&m_roomsManager);

				auto client = std::make_shared<Main::Network::Session>(m_scheduler, std::move(*MainServer::m_socket),
					std::bind(&Main::Network::SessionsManager::removeSession, &m_sessionsManager, std::placeholders::_1));

				client->sendConnectionACK(Common::Enums::MAIN_SERVER);
				asyncAccept();
			});
	}

	void MainServer::initializeAllCommands()
	{
 		m_chatCommands.addSimpleCommand("setcurrency", std::make_unique<Main::Command::SetMaxCurrency>(Common::Enums::GRADE_NORMAL));
		m_chatCommands.addSimpleCommand("online", std::make_unique<Main::Command::OnlineCommand>(Common::Enums::GRADE_TESTER));
		m_chatCommands.addSimpleCommand("shutdown", std::make_unique<Main::Command::Shutdown>(Common::Enums::GRADE_TESTER));
		m_chatCommands.addComplexCommand("setlevel", std::make_unique<Main::Command::SetLevel>(Common::Enums::GRADE_MOD));
		m_chatCommands.addComplexCommand("setname", std::make_unique<Main::Command::SetNickname>(Common::Enums::GRADE_MOD));
		m_chatCommands.addComplexCommand("disconnect", std::make_unique<Main::Command::TiltClient>(Common::Enums::GRADE_MOD));
		m_chatCommands.addComplexCommand("announce", std::make_unique<Main::Command::CreateAnnouncement>(Common::Enums::GRADE_TESTER));
		m_chatCommands.addComplexCommand("tip", std::make_unique<Main::Command::CreateAnnouncement>(Common::Enums::GRADE_TESTER));
		m_chatCommands.addComplexCommand("ban", std::make_unique<Main::Command::BanPlayer>(Common::Enums::GRADE_TESTER));
		m_chatCommands.addComplexCommand("mute", std::make_unique<Main::Command::MutePlayer>(Common::Enums::GRADE_MOD));
		m_chatCommands.addComplexCommand("unmute", std::make_unique<Main::Command::UnmutePlayer>(Common::Enums::GRADE_MOD));
		m_chatCommands.addComplexCommand("muteinfo", std::make_unique<Main::Command::MuteInfo>(Common::Enums::GRADE_MOD));
		m_chatCommands.addComplexCommand("getitem", std::make_unique<Main::Command::SpawnItem>(Common::Enums::GRADE_NORMAL));
		m_chatCommands.addComplexRoomCommand("kick", std::make_unique<Main::Command::KickPlayer>(Common::Enums::GRADE_MOD));
		m_chatCommands.addSimpleRoomCommand("breakroom", std::make_unique<Main::Command::Breakroom>(Common::Enums::GRADE_MOD));
		m_chatCommands.addComplexRoomCommand("changehost", std::make_unique<Main::Command::ChangeHost>(Common::Enums::GRADE_TESTER));
		m_chatCommands.addSimpleRoomCommand("muteroom", std::make_unique<Main::Command::Muteroom>(Common::Enums::GRADE_MOD));
		m_chatCommands.addSimpleRoomCommand("unmuteroom", std::make_unique<Main::Command::UnmuteRoom>(Common::Enums::GRADE_MOD));
		m_chatCommands.addDatabaseCommand("unban", std::make_unique<Main::Command::UnbanAccount>(Common::Enums::GRADE_TESTER));
		m_chatCommands.addDatabaseCommand("addplayer", std::make_unique<Main::Command::AddPlayer>(Common::Enums::GRADE_TESTER));
	}
}
