#include "../include/CastServer.h"
#include "Network/Session.h"
#include "../include/Handlers/CastConnectionHandler.h"
#include "../include/Handlers/CastServerPongHandler.h"
#include "../include/Handlers/RespawnHandler.h"
#include "../include/Handlers/InitialMatchLoadingHandler.h"
#include "../include/Handlers/AccountNamesHandler.h"
#include "../include/Handlers/UnknownHandler1.h"
#include "../include/Handlers/RoomTickProviderHandler.h"
#include "../include/Handlers/UnknownHandlers.h"
#include "../include/Handlers/PlayerPositionHandler.h"
#include "../include/Handlers/WeaponHandlers/ExplosivesHandler.h"
#include "../include/Handlers/WeaponHandlers/ExplosivesDamageHandler.h"
#include "../include/Handlers/WeaponHandlers/WeaponReloadHandler.h"
#include "../include/Handlers/WeaponHandlers/WeaponAttackHandler.h"
#include "../include/Handlers/SwitchStateHandler.h"
#include "../include/Handlers/RoomCreationHandler.h"
#include "../include/Handlers/HostRoomJoinControlHandler.h"
#include "../include/Handlers/RoomTickSyncHandler.h"
#include "../include/Handlers/PlayerRoomSyncHandler.h"
#include "../include/Handlers/EliminationNextRoundHandler.h"
#include "../include/Handlers/MatchLeaveHandler.h"
#include "../include/Handlers/CrashHandler.h"
#include "../include/Handlers/ZombieModeHandlers.h"
#include "../include/Handlers/MatchEndHandler.h"


namespace Cast
{
	CastServer::CastServer(ioContext& io_context, std::uint16_t port, std::uint16_t serverId)
		: m_io_context{ io_context }
		, m_acceptor{ io_context, tcp::endpoint(tcp::v4(), port), }
		, m_serverId{ serverId }
	{
		// WORKS:
		Common::Network::Session::addCallback<Cast::Network::Session>(71, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::pongHandler(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Cast::Network::Session>(136, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handlePlayerJoinRoom(request, session, m_roomsManager); });

		// mg
		Common::Network::Session::addCallback<Cast::Network::Session>(143, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::nonHostWeaponAttack(request, session, m_roomsManager); });

		// melee
		Common::Network::Session::addCallback<Cast::Network::Session>(145, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::nonHostWeaponAttack(request, session, m_roomsManager); });

		// bazooka
		Common::Network::Session::addCallback<Cast::Network::Session>(147, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::nonHostWeaponAttack(request, session, m_roomsManager); });

		// rifle
		Common::Network::Session::addCallback<Cast::Network::Session>(148, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::nonHostWeaponAttack(request, session, m_roomsManager); });

		// shotgun
		Common::Network::Session::addCallback<Cast::Network::Session>(149, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::nonHostWeaponAttack(request, session, m_roomsManager); });

		// sniper
		Common::Network::Session::addCallback<Cast::Network::Session>(150, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::nonHostWeaponAttack(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Cast::Network::Session>(162, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handlePlayerRespawnRequest(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Cast::Network::Session>(277, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleRoomCreation(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Cast::Network::Session>(279, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::leaveRoom(request, session, m_roomsManager); });

		// OK
		Common::Network::Session::addCallback<Cast::Network::Session>(252, Cast::Handlers::connectionHandler);
		
		Common::Network::Session::addCallback<Cast::Network::Session>(264, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleExplosivesDamage(request, session, m_roomsManager); });

		// mg 
		Common::Network::Session::addCallback<Cast::Network::Session>(266, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleHostWeaponAttack(request, session, m_roomsManager); });

		// melee
		Common::Network::Session::addCallback<Cast::Network::Session>(267, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleHostWeaponAttack(request, session, m_roomsManager); });

		// rifle
		Common::Network::Session::addCallback<Cast::Network::Session>(268, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleHostWeaponAttack(request, session, m_roomsManager); });

		// shotgun
		Common::Network::Session::addCallback<Cast::Network::Session>(269, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleHostWeaponAttack(request, session, m_roomsManager); });

		// sniper
		Common::Network::Session::addCallback<Cast::Network::Session>(270, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleHostWeaponAttack(request, session, m_roomsManager); });
		
		Common::Network::Session::addCallback<Cast::Network::Session>(272, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleExplosives(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Cast::Network::Session>(275, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleWeaponReload(request, session); });

		Common::Network::Session::addCallback<Cast::Network::Session>(281, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handlePlayerPosition(request, session, m_roomsManager); });
		
		Common::Network::Session::addCallback<Cast::Network::Session>(253, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleCrash(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Cast::Network::Session>(254, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleMatchEnd(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Cast::Network::Session>(256, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleMatchLeave(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Cast::Network::Session>(284, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleAccountNames(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Cast::Network::Session>(309, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handlePlayerSyncWithRoom(request, session, m_roomsManager); }); 

		// without this e.g. the bomb in "bomb battle" doesn't exist seemingly
		Common::Network::Session::addCallback<Cast::Network::Session>(92, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::playerToHost(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Cast::Network::Session>(79, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleRoomTickSyncRequest(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Cast::Network::Session>(306, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::unknownHandler306(request, session, m_roomsManager); });

		// CTB battery respawn -- NON host -- Seems OK
		Common::Network::Session::addCallback<Cast::Network::Session>(90, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::similarHandlers(request, session, m_roomsManager); });

		// CTB battery respawn -- host -- Seems OK
		Common::Network::Session::addCallback<Cast::Network::Session>(161, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::similarHandlers(request, session, m_roomsManager); });

		// Bomb Battle related -- NON host
		Common::Network::Session::addCallback<Cast::Network::Session>(151, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::playerToHost(request, session, m_roomsManager); });

		// Bomb battle drop bomb -- OK
		Common::Network::Session::addCallback<Cast::Network::Session>(152, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::playerToHost(request, session, m_roomsManager); });

		// Bomb battle -- host -- OK
		Common::Network::Session::addCallback<Cast::Network::Session>(271, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::similarHandlers(request, session, m_roomsManager); });

		Common::Network::Session::addCallback<Cast::Network::Session>(255, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::roomInfoHandler(request, session, m_roomsManager); });

		// Seems ok?
		Common::Network::Session::addCallback<Cast::Network::Session>(258, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleMatchStart(request, session, m_roomsManager); });

		// OK
		Common::Network::Session::addCallback<Cast::Network::Session>(257, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleMatchInitialLoading(request, session, m_roomsManager); });

		// OK
		Common::Network::Session::addCallback<Cast::Network::Session>(276, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handlePlayerRespawn(request, session, m_roomsManager); });

		// OK
		Common::Network::Session::addCallback<Cast::Network::Session>(408, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleRoomTickProviding(request, session, m_roomsManager); });


		// OK
		Common::Network::Session::addCallback<Cast::Network::Session>(260, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleItemRespawn(request, session, m_roomsManager); });

		// OK
		Common::Network::Session::addCallback<Cast::Network::Session>(261, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleZombieRespawn(request, session, m_roomsManager); });

		// OK
		Common::Network::Session::addCallback<Cast::Network::Session>(262, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleItemPickup<Common::Enums::HOST>(request, session, m_roomsManager); });

		// OK
		Common::Network::Session::addCallback<Cast::Network::Session>(263, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleZombieAbility<Common::Enums::HOST>(request, session, m_roomsManager); });

		// OK
		Common::Network::Session::addCallback<Cast::Network::Session>(94, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleItemPickup<Common::Enums::NON_HOST>(request, session, m_roomsManager); });

		// OK
		Common::Network::Session::addCallback<Cast::Network::Session>(98, [&](const Common::Network::Packet& request,
			Cast::Network::Session& session) { Cast::Handlers::handleZombieAbility<Common::Enums::NON_HOST>(request, session, m_roomsManager); });
	}

		

	void CastServer::asyncAccept()
	{
		m_socket.emplace(m_io_context);
		m_acceptor.async_accept(*m_socket, [&](asio::error_code error)
			{
				std::cout << "CAST: ASYNC ACCEPTED\n";
				m_sessionsManager.setRoomsManager(&m_roomsManager);

				auto client = std::make_shared<Cast::Network::Session>(std::move(*CastServer::m_socket),
					std::bind(&Cast::Network::SessionsManager::removeSession, &m_sessionsManager, std::placeholders::_1));

				client->sendConnectionACK(Common::Enums::CAST_SERVER);

				asyncAccept();
			});
	}
}
