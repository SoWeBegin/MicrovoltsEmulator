#ifndef AUTH_PLAYER_MANAGER_H
#define AUTH_PLAYER_MANAGER_H

#include "../Structures/AuthAccountInfo.h"
#include "Network/Session.h"
#include <unordered_map>
#include <memory>
#include <cstddef>

namespace Auth
{
    namespace Player
    {
        class AuthPlayerManager
        {
        private:
            using AccountInfo = Auth::Structures::BasicAccountInfo;
            std::unordered_map<std::size_t, AccountInfo> m_players;

            AuthPlayerManager() {}

        public:
            AuthPlayerManager(const AuthPlayerManager&) = delete;
            AuthPlayerManager& operator=(const AuthPlayerManager&) = delete;

            static AuthPlayerManager& getInstance()
            {
                static AuthPlayerManager instance;
                return instance;
            }

            void addPlayer(std::size_t sessionId, const AccountInfo& playerInfo)
            {
                m_players.emplace(sessionId, playerInfo);
            }

            void remove(std::size_t id)
            {
                if (m_players.contains(id))
                {
                    m_players.erase(id);
                }
            }
        };
    }
}

#endif
