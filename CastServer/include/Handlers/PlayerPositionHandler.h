#ifndef PLAYER_POSITION_HANDLER_H
#define PLAYER_POSITION_HANDLER_H

#include "../Network/CastSession.h"
#include "../Network/SessionsManager.h"
#include "../Structures/PlayerPositionFromServer.h"
#include "../Structures/SuicideStruct.h"
#include "../Utils/HostSuicideUtils.h"

#include "Utils/Logger.h"

namespace Cast
{
    namespace Handlers
    {
        inline void handlePlayerPosition(const Common::Network::Packet& request, Cast::Network::Session& session, Cast::Classes::RoomsManager& roomsManager)
        {
            using namespace Cast::Structures;

            Cast::Structures::ClientPlayerInfoBasic playerPositionFromClient{};
            std::memcpy(&playerPositionFromClient, request.getData(), sizeof(playerPositionFromClient));

            Common::Network::Packet response;
            response.setTcpHeader(session.getId(), Common::Enums::NO_ENCRYPTION);
            response.setOrder(322);
            response.setOption(1);
            const auto fullSize = request.getFullSize();

            if (fullSize == 36)
            {
                Cast::Structures::ClientPlayerInfoBullet playerPositionBullet{};
                std::memcpy(&playerPositionBullet, request.getData(), sizeof(playerPositionBullet));

                PlayerInfoResponseWithBullets playerInfoResponseWithBullets;
                playerInfoResponseWithBullets.specificInfo.enableBullet = true;

                playerInfoResponseWithBullets.tick = playerPositionFromClient.matchTick;
                playerInfoResponseWithBullets.position = playerPositionFromClient.position;
                playerInfoResponseWithBullets.direction = playerPositionFromClient.direction;
                playerInfoResponseWithBullets.specificInfo.animation1 = playerPositionFromClient.animation1;
                playerInfoResponseWithBullets.specificInfo.animation2 = playerPositionFromClient.animation2;
                playerInfoResponseWithBullets.rotation1 = request.getExtra();
                playerInfoResponseWithBullets.rotation2 = request.getOption();
                playerInfoResponseWithBullets.rotation3 = playerPositionFromClient.rotation;
                playerInfoResponseWithBullets.specificInfo.sessionId = static_cast<std::uint32_t>(session.getId());
                playerInfoResponseWithBullets.bullets = playerPositionBullet.bulletStruct;
                playerInfoResponseWithBullets.currentWeapon = playerPositionBullet.bulletStruct.bullet4;

                response.setData(reinterpret_cast<std::uint8_t*>(&playerInfoResponseWithBullets), sizeof(playerInfoResponseWithBullets));
            }
            else if (request.getFullSize() == 40)
            {
                Cast::Structures::ClientPlayerInfoComplete playerPositionComplete{};
                std::memcpy(&playerPositionComplete, request.getData(), request.getDataSize());

                Cast::Structures::ClientPlayerInfoBullet playerPositionBullet{};
                std::memcpy(&playerPositionBullet, request.getData(), sizeof(playerPositionBullet));

                PlayerInfoResponseWithBullets playerInfoResponseWithBullets;
                playerInfoResponseWithBullets.specificInfo.enableBullet = true;

                playerInfoResponseWithBullets.tick = playerPositionFromClient.matchTick;
                playerInfoResponseWithBullets.position = playerPositionFromClient.position;
                playerInfoResponseWithBullets.direction = playerPositionFromClient.direction;
                playerInfoResponseWithBullets.specificInfo.animation1 = playerPositionFromClient.animation1;
                playerInfoResponseWithBullets.specificInfo.animation2 = playerPositionFromClient.animation2;
                playerInfoResponseWithBullets.rotation1 = request.getExtra();
                playerInfoResponseWithBullets.rotation2 = request.getOption();
                playerInfoResponseWithBullets.rotation3 = playerPositionFromClient.rotation;
                playerInfoResponseWithBullets.specificInfo.sessionId = static_cast<std::uint32_t>(session.getId());
                playerInfoResponseWithBullets.bullets = playerPositionBullet.bulletStruct;
                playerInfoResponseWithBullets.currentWeapon = playerPositionBullet.bulletStruct.bullet4;

                PlayerInfoResponseComplete playerInfoResponseComplete{ playerInfoResponseWithBullets };
                playerInfoResponseComplete.playerInfoBasicResponse.specificInfo.enableJump = true;

                playerInfoResponseComplete.jump = playerPositionComplete.jumpStruct;
                response.setData(reinterpret_cast<std::uint8_t*>(&playerInfoResponseComplete), sizeof(playerInfoResponseComplete));
            }
            else
            {
                PlayerInfoBasicResponse playerInfoBasicResponse;
                playerInfoBasicResponse.tick = playerPositionFromClient.matchTick;
                playerInfoBasicResponse.position = playerPositionFromClient.position;
                playerInfoBasicResponse.direction = playerPositionFromClient.direction;
                playerInfoBasicResponse.currentWeapon = playerPositionFromClient.weapon;
                playerInfoBasicResponse.specificInfo.animation1 = playerPositionFromClient.animation1;
                playerInfoBasicResponse.specificInfo.animation2 = playerPositionFromClient.animation2;
                playerInfoBasicResponse.rotation1 = request.getExtra();
                playerInfoBasicResponse.rotation2 = request.getOption();
                playerInfoBasicResponse.rotation3 = playerPositionFromClient.rotation;
                playerInfoBasicResponse.specificInfo.sessionId = static_cast<std::uint32_t>(session.getId());

               /* if ((session.getRoomId() == session.getId()) && Cast::Utils::isSuicide(roomsManager.getMapOf(session.getRoomId()), playerInfoBasicResponse.position.positionZ))
                {
                    response.setOrder(264);
                    Cast::Structures::SuicideStructure suicideStruct;
                    suicideStruct.uniqueId = Main::Structures::UniqueId{ static_cast<std::uint32_t>(session.getId()), 4, 0 };
                    suicideStruct.posX = playerPositionFromClient.position.positionX;
                    suicideStruct.posY = playerPositionFromClient.position.positionY;
                    suicideStruct.posZ = playerPositionFromClient.position.positionZ;
                    response.setData(reinterpret_cast<std::uint8_t*>(&suicideStruct), sizeof(suicideStruct));
                    roomsManager.broadcastToRoom(session.getRoomId(), response);
                    return;
                }*/
                if (fullSize == 28)
                {
                    response.setData(reinterpret_cast<std::uint8_t*>(&playerInfoBasicResponse), sizeof(playerInfoBasicResponse));
                }
                else if (fullSize == 32)
                {
                    playerInfoBasicResponse.specificInfo.enableJump = true;
                    PlayerInfoResponseWithJump playerInfoResponseWithJump{ playerInfoBasicResponse };

                    Cast::Structures::ClientPlayerInfoJump playerPositionJump{};
                    std::memcpy(&playerPositionJump, request.getData(), request.getDataSize());

                    playerInfoResponseWithJump.jump = playerPositionJump.jumpStruct;
                    response.setData(reinterpret_cast<std::uint8_t*>(&playerInfoResponseWithJump), sizeof(playerInfoResponseWithJump));
                }
            }

            roomsManager.broadcastToRoomExceptSelf(session.getId(), response);
        }
    }
}

#endif