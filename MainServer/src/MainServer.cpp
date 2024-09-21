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
#include "../include/Handlers/Player/PlayerStateHandler.h"
#include "../include/Handlers/Player/PingHandler.h"
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
#include "../include/Handlers/CapsuleReqHandler.h"

#include "../include/Network/MainSession.h"
#include "../include/ChatCommands/AllCommandsIncludes.h"
#include "../include/Boxes/BrilliantBox.h"
#include "../include/Boxes/MpBox.h"
#include "../include/Network/AuthSession.h"


namespace Main
{
	MainServer::MainServer(ioContext& io_context, std::uint16_t clientPort, std::uint16_t authPort, std::uint16_t serverId)
		: m_io_context{ io_context }
		, m_acceptor{ io_context, tcp::endpoint(tcp::v4(), clientPort) } 
		, m_authServerAcceptor{ io_context, tcp::endpoint(tcp::v4(), authPort) }
		, m_serverId{ serverId }
		, m_database{ "../ExternalLibraries/Database/GameDatabase.db" }
		, m_scheduler{ 5, m_database }
	{
		const auto durationSinceEpoch = std::chrono::system_clock::now().time_since_epoch();
		m_timeSinceLastRestart = static_cast<std::uint64_t>(duration_cast<std::chrono::milliseconds>(durationSinceEpoch).count());

		initializeAllCommands();
		initializeBoxes();
		
		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(52, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handlePlayerBlock(request, session, m_sessionsManager, m_database); });

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(53, Main::Handlers::handlePlayerUnblock);

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(54, Main::Handlers::handleBlockedPlayerList); // OK

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(57, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleLobbyClanList(request, session, m_sessionsManager); });

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(61, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleGeneralFriendRequests(request, session, m_sessionsManager, m_database); });

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(62, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleFriendDeletion(request, session, m_sessionsManager); });

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(63, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleFriendList(request, session, m_sessionsManager, m_database); });

		//Common::Network::Session::addCallback<Main::Network::Session>(66, Main::Handlers::handleMailboxGiftSend);

		//Common::Network::Session::addCallback<Main::Network::Session>(67, Main::Handlers::handleMailboxGiftDisplay);
		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(68, [&](const Common::Network::Packet& request, 
			Main::Network::Session& session) { Main::Handlers::handleInitialPlayerInfos(request, session, m_sessionsManager, m_database, m_timeSinceLastRestart); });

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(71, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handlePing(request, session, m_roomsManager); });

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(74, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleCharacterSelection(request, session, m_roomsManager); });

		//Common::Network::Session::addCallback<Main::Network::Session>(83, [&](const Common::Network::Packet& request,
			//Main::Network::Session& session) { Main::Handlers::handleCapsuleReq(request, session, m_roomsManager); });

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(84, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleLobbyUserList(request, session, m_sessionsManager); });

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(85, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleLobbyAccountInfo(request, session, m_sessionsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(89, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleBattery(request, session, m_roomsManager); });

		// CHECK THIS!
		Common::Network::Session::addCallback<Main::Network::Session>(87, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleEquippedItemSwitch(request, session, m_roomsManager); });

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(86, Main::Handlers::handleBoughtItem);

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(88, Main::Handlers::handleItemDelete);

		// Not working yet: on previous versions the implementation was completely different!
		//Common::Network::Session::addCallback<Main::Network::Session>(98, Main::Handlers::handleCapsuleSpin);

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(96, Main::Handlers::handleItemRefund);

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(97, Main::Handlers::handleItemUpgrade);

		Common::Network::Session::addCallback<Main::Network::Session>(98, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleGeneralItem(request, session, m_database, m_boxes); });


		Common::Network::Session::addCallback<Main::Network::Session>(99, Main::Handlers::handleMailboxDelete);

		Common::Network::Session::addCallback<Main::Network::Session>(100, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleMailboxCommunication(request, session, m_sessionsManager, m_database); });

		Common::Network::Session::addCallback<Main::Network::Session>(101, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleReadMailbox(request, session, m_database); });

		Common::Network::Session::addCallback<Main::Network::Session>(102, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleMailboxDisplay(request, session, m_sessionsManager, m_database); });

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(103, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomStart(request, session, m_roomsManager, m_timeSinceLastRestart); }); 

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(120, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleSimpleRoomSetting<Main::Enums::SETTING_ITEM>(request, session, m_roomsManager); }); // Item on/off

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(121, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomMiscellaneous(request, session, m_roomsManager, m_timeSinceLastRestart); }); 
		// Mode, Team Balance, Weapon Restriction, map, votekick

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(122, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomMiscellaneous(request, session, m_roomsManager, m_timeSinceLastRestart); });

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(123, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleSimpleRoomSetting<Main::Enums::SETTING_OPEN>(request, session, m_roomsManager); }); // open on/off

		// OK

		Common::Network::Session::addCallback<Main::Network::Session>(124, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleHostChange(request, session, m_roomsManager); }); 

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(125, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomMiscellaneous(request, session, m_roomsManager, m_timeSinceLastRestart); }); // Password

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(126, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomMiscellaneous(request, session, m_roomsManager, m_timeSinceLastRestart); }); // Title,

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(127, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleSimpleRoomSetting<Main::Enums::SETTING_MAP>(request, session, m_roomsManager); }); // Map (normal click)

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(128, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleSimpleRoomSetting<Main::Enums::SETTING_PLAYERS_PER_TEAM>(request, session, m_roomsManager); }); // NvsN setting

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(129, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleSimpleRoomSetting<Main::Enums::SETTING_OBSERVER>(request, session, m_roomsManager); }); // ObserverMode,

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(130, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleSimpleRoomSetting<Main::Enums::SETTING_SPECIFIC>(request, session, m_roomsManager); }); // total kills/rounds...

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(131, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleSimpleRoomSetting<Main::Enums::SETTING_TIME>(request, session, m_roomsManager); });  // time
			
		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(134, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomCreation(request, session, m_roomsManager); });

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(136, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomJoin(request, session, m_roomsManager); });
		
		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(137, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomLeave(request, session, m_sessionsManager, m_roomsManager); });

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(138, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomsList(request, session, m_roomsManager); });

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(154, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handlePlayerState(request, session, m_roomsManager); });
			
		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(155, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomMiscellaneous(request, session, m_roomsManager, m_timeSinceLastRestart); }); // Team switch

		// OK
		Common::Network::Session::addCallback<Main::Network::Session>(158, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleLobbyChatMessage(request, session, m_sessionsManager, m_chatCommands, m_roomsManager, m_database); });

		Common::Network::Session::addCallback<Main::Network::Session>(157, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleRoomChatMessage(request, session, m_sessionsManager, m_chatCommands, m_roomsManager, m_database); });

		/*
		Common::Network::Session::addCallback<Main::Network::Session>(202, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleModeEvents(request, session, m_database); });

		Common::Network::Session::addCallback<Main::Network::Session>(232, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleMapEvents(request, session, m_database); });
*/

		// same order,ok (impl diff not checked yet!!)
		Common::Network::Session::addCallback<Main::Network::Session>(256, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleMatchLeave(request, session, m_sessionsManager, m_roomsManager); });
		
		// same order,ok (impl diff not checked yet!!)
		Common::Network::Session::addCallback<Main::Network::Session>(259, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleEliminationNextRound(request, session, m_roomsManager); });

		// CTB respawn (recheck??)
		Common::Network::Session::addCallback<Main::Network::Session>(156, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::unknown(request, session, m_roomsManager, m_sessionsManager); });

		// Bomb Battle for host (unsure??)  && room invite ??
		Common::Network::Session::addCallback<Main::Network::Session>(159, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::unknown(request, session, m_roomsManager, m_sessionsManager); });

		Common::Network::Session::addCallback<Main::Network::Session>(104, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleEliminationNextRound2(request, session, m_roomsManager); });
		
		// same order,ok (impl diff not checked yet!!)
		Common::Network::Session::addCallback<Main::Network::Session>(254, [&](const Common::Network::Packet& request,
			Main::Network::Session& session) { Main::Handlers::handleMatchEnding(request, session, m_roomsManager); });
	}

	void MainServer::asyncAccept()
	{
		m_socket.emplace(m_io_context);
		m_acceptor.async_accept(*m_socket, [&](asio::error_code error)
			{
				m_sessionsManager.setRoomsManager(&m_roomsManager);

				auto client = std::make_shared<Main::Network::Session>(m_scheduler, std::move(*MainServer::m_socket),
					std::bind(&Main::Network::SessionsManager::removeSession, &m_sessionsManager, std::placeholders::_1));

				client->sendConnectionACK(Common::Enums::MAIN_SERVER);
				asyncAccept();
			});
	}

	void MainServer::asyncAcceptAuthServer() 
	{
		m_authSocket.emplace(m_io_context);
		m_authServerAcceptor.async_accept(*m_authSocket, [this](asio::error_code error) 
			{
			if (!error) {
				auto authSession = std::make_shared<Main::Network::AuthSession>(std::move(*m_authSocket), m_sessionsManager);
				authSession->start();
			}
			else {
				std::cerr << "Error during auth server accept: " << error.message() << std::endl;
			}

			asyncAcceptAuthServer();
			});
	}



	void MainServer::initializeAllCommands()
	{
 		m_chatCommands.addSimpleCommand("setcurrency", std::make_unique<Main::Command::SetMaxCurrency>(Common::Enums::GRADE_NORMAL));
		m_chatCommands.addSimpleCommand("online", std::make_unique<Main::Command::OnlineCommand>(Common::Enums::GRADE_GM));
		m_chatCommands.addSimpleCommand("shutdown", std::make_unique<Main::Command::Shutdown>(Common::Enums::GRADE_GM));
		m_chatCommands.addComplexCommand("setlevel", std::make_unique<Main::Command::SetLevel>(Common::Enums::GRADE_MOD));
		m_chatCommands.addComplexCommand("setname", std::make_unique<Main::Command::SetNickname>(Common::Enums::GRADE_MOD));
		m_chatCommands.addComplexCommand("disconnect", std::make_unique<Main::Command::TiltClient>(Common::Enums::GRADE_MOD));
		m_chatCommands.addComplexCommand("announce", std::make_unique<Main::Command::CreateAnnouncement>(Common::Enums::GRADE_GM));
		m_chatCommands.addComplexCommand("tip", std::make_unique<Main::Command::CreateAnnouncement>(Common::Enums::GRADE_GM));
		m_chatCommands.addComplexCommand("ban", std::make_unique<Main::Command::BanPlayer>(Common::Enums::GRADE_MOD));
		m_chatCommands.addComplexCommand("mute", std::make_unique<Main::Command::MutePlayer>(Common::Enums::GRADE_MOD));
		m_chatCommands.addComplexCommand("unmute", std::make_unique<Main::Command::UnmutePlayer>(Common::Enums::GRADE_MOD));
		m_chatCommands.addComplexCommand("muteinfo", std::make_unique<Main::Command::MuteInfo>(Common::Enums::GRADE_MOD));
		m_chatCommands.addComplexCommand("getitem", std::make_unique<Main::Command::SpawnItem>(Common::Enums::GRADE_NORMAL));
		m_chatCommands.addComplexRoomCommand("kick", std::make_unique<Main::Command::KickPlayer>(Common::Enums::GRADE_MOD));
		m_chatCommands.addSimpleRoomCommand("breakroom", std::make_unique<Main::Command::Breakroom>(Common::Enums::GRADE_MOD));
		m_chatCommands.addComplexRoomCommand("changehost", std::make_unique<Main::Command::ChangeHost>(Common::Enums::GRADE_MOD));
		m_chatCommands.addSimpleRoomCommand("muteroom", std::make_unique<Main::Command::Muteroom>(Common::Enums::GRADE_MOD));
		m_chatCommands.addSimpleRoomCommand("unmuteroom", std::make_unique<Main::Command::UnmuteRoom>(Common::Enums::GRADE_MOD));
		m_chatCommands.addDatabaseCommand("unban", std::make_unique<Main::Command::UnbanAccount>(Common::Enums::GRADE_MOD));
		m_chatCommands.addDatabaseCommand("addplayer", std::make_unique<Main::Command::AddPlayer>(Common::Enums::GRADE_GM));
	}

	void MainServer::initializeBoxes()
	{
		m_boxes[Main::Enums::BoxIds::BRILLIANT_HAMMER] = std::make_unique<Main::Box::BrilliantBox>(Main::Enums::BoxItemsCommon::WHITE_HAMMER);
		m_boxes[Main::Enums::BoxIds::BRILLIANT_SWORD] = std::make_unique<Main::Box::BrilliantBox>(Main::Enums::BoxItemsCommon::WHITE_KATANA);
		m_boxes[Main::Enums::BoxIds::BRILLIANT_PENCIL] = std::make_unique<Main::Box::BrilliantBox>(Main::Enums::BoxItemsCommon::WHITE_PENCIL);
		m_boxes[Main::Enums::BoxIds::BRILLIANT_SNIPER] = std::make_unique<Main::Box::BrilliantBox>(Main::Enums::BoxItemsCommon::WHITE_SEAWASP);
		m_boxes[Main::Enums::BoxIds::BRILLIANT_BAZOOKA] = std::make_unique<Main::Box::BrilliantBox>(Main::Enums::BoxItemsCommon::OUTLAW);
		m_boxes[Main::Enums::BoxIds::BRILLIANT_GRENADE] = std::make_unique<Main::Box::BrilliantBox>(Main::Enums::BoxItemsCommon::WHITE_PREDATOR);
		m_boxes[Main::Enums::BoxIds::BRILLIANT_KUKRI] = std::make_unique<Main::Box::BrilliantBox>(Main::Enums::BoxItemsCommon::GOLDEN_KUKRI);
		m_boxes[Main::Enums::BoxIds::BRILLIANT_MENTHA] = std::make_unique<Main::Box::BrilliantBox>(Main::Enums::BoxItemsCommon::MENTHA);
		m_boxes[Main::Enums::BoxIds::BRILLIANT_CORAL] = std::make_unique<Main::Box::BrilliantBox>(Main::Enums::BoxItemsCommon::GOLD_CORAL);
		m_boxes[Main::Enums::BoxIds::BRILLIANT_BLASTER] = std::make_unique<Main::Box::BrilliantBox>(Main::Enums::BoxItemsCommon::GOLD_CORAL);

		m_boxes[Main::Enums::MP_100] = std::make_unique<Main::Box::MpBox>(100);
		m_boxes[Main::Enums::MP_500] = std::make_unique<Main::Box::MpBox>(500);
		m_boxes[Main::Enums::MP_1000] = std::make_unique<Main::Box::MpBox>(1000);
		m_boxes[Main::Enums::MP_2500] = std::make_unique<Main::Box::MpBox>(2500);
		m_boxes[Main::Enums::MP_3000] = std::make_unique<Main::Box::MpBox>(3000);
		m_boxes[Main::Enums::MP_4000] = std::make_unique<Main::Box::MpBox>(4000);
		m_boxes[Main::Enums::MP_5000] = std::make_unique<Main::Box::MpBox>(5000);
		m_boxes[Main::Enums::MP_7000] = std::make_unique<Main::Box::MpBox>(7000);
		m_boxes[Main::Enums::MP_10000] = std::make_unique<Main::Box::MpBox>(10000);
		m_boxes[Main::Enums::MP_20000] = std::make_unique<Main::Box::MpBox>(20000);
		m_boxes[Main::Enums::MP_50000] = std::make_unique<Main::Box::MpBox>(50000);
		m_boxes[Main::Enums::MP_100000] = std::make_unique<Main::Box::MpBox>(100000);
		m_boxes[Main::Enums::MP_500000] = std::make_unique<Main::Box::MpBox>(500000);


	}
}
