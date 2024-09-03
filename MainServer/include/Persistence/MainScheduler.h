#ifndef MAIN_SCHEDULER_HEADER
#define MAIN_SCHEDULER_HEADER

#include <unordered_map>
#include <map>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>

#include "../Structures/AccountInfo/MainAccountInfo.h"
#include "../Structures/Item/MainItem.h"
#include "../Structures/Item/MainEquippedItem.h"
#include "../Persistence/MainDatabaseManager.h"

namespace Main
{
    namespace Persistence
    {
        class MainScheduler
        {
        private:
            using AccountInfo = Main::Structures::AccountInfo;
            using Item = Main::Structures::Item;
            using EquippedItem = Main::Structures::EquippedItem;
            using BoughtItem = Main::Structures::BoughtItem;

            std::size_t m_wakeupFrequency{};
            std::thread m_schedulerThread{};
            bool m_stopRequested = false;
            std::mutex m_mutex{};

            Main::Persistence::PersistentDatabase& m_database;

            // [accountId][updateName][callback]
            std::unordered_map<std::uint32_t, std::map<std::size_t, std::function<void()>>> m_databaseCallbacks{};

            std::unordered_map<std::uint32_t, std::map<std::size_t, std::function<void()>>> m_databaseCallbacksIncremental{};
            std::size_t m_incrementalDifferentiationKey = 0;


        public:
            explicit MainScheduler(std::size_t wakeupFrequency, Main::Persistence::PersistentDatabase& database);

            ~MainScheduler();

            template<typename Function, typename... Args>
            void addCallback(std::uint32_t accountId, std::size_t differentiationKey, Function databaseMemberFunction, Args&&... args)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_databaseCallbacks[accountId][differentiationKey] = std::bind(databaseMemberFunction, &m_database, std::forward<Args>(args)...);
            }

            template<typename Function, typename... Args>
            void addRepetitiveCallback(std::uint32_t accountId, Function databaseMemberFunction, Args&&... args)
            {
                std::lock_guard<std::mutex> lock(m_mutex); 
                m_databaseCallbacksIncremental[accountId][++m_incrementalDifferentiationKey] = std::bind(databaseMemberFunction, &m_database, std::forward<Args>(args)...);
            }

            template<typename Function, typename... Args>
            void immediatePersist(std::uint32_t accountId, Function databaseMemberFunction, Args&&... args)
            {
                std::lock_guard<std::mutex> lock(m_mutex); 
                std::bind(databaseMemberFunction, &m_database, std::forward<Args>(args)...)();
            }

        private:
            void schedulerLoop();

            void persist();

        public:
            void persistFor(std::uint32_t accountId);
        };
    }
}
#endif
