
#include <unordered_map>
#include <map>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>

#include "../../include/Structures/AccountInfo/MainAccountInfo.h"
#include "../../include/Structures/Item/MainItem.h"
#include "../../include/Structures/Item/MainEquippedItem.h"
#include "../../include/Persistence/MainDatabaseManager.h"
#include "../../include/Persistence/MainScheduler.h"

namespace Main
{
    namespace Persistence
    {
        MainScheduler::MainScheduler(std::size_t wakeupFrequency, Main::Persistence::PersistentDatabase& database)
            : m_wakeupFrequency{ wakeupFrequency }
            , m_database{ database }
        {
            m_schedulerThread = std::thread(&MainScheduler::schedulerLoop, this);
        }

        MainScheduler::~MainScheduler()
        {
            if (m_schedulerThread.joinable())
            {
                m_stopRequested = true;
                m_schedulerThread.join();
            }
        }

        void  MainScheduler::schedulerLoop()
        {
            while (!m_stopRequested)
            {
                std::this_thread::sleep_for(std::chrono::seconds(m_wakeupFrequency));
                persist();
            }
        }

        void  MainScheduler::persist()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (const auto& [accountId, callbacks] : m_databaseCallbacksIncremental)
            {
                for (const auto& [updateType, callback] : callbacks)
                {
                    callback();
                }
            }

            for (const auto& [accountId, callbacks] : m_databaseCallbacks)
            {
                for (const auto& [updateType, callback] : callbacks)
                {
                    callback();
                }
            }

            m_databaseCallbacks.clear();
            m_databaseCallbacksIncremental.clear();
            m_incrementalDifferentiationKey = 0;
        }

        void MainScheduler::persistFor(std::uint32_t accountId)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (const auto& [updateType, callback] : m_databaseCallbacksIncremental[accountId])
            {
                callback();
            }

            for (const auto& [updateType, callback] : m_databaseCallbacks[accountId])
            {
                callback();
            }

            m_databaseCallbacks.erase(accountId);
            m_databaseCallbacksIncremental.erase(accountId);
            m_incrementalDifferentiationKey = 0;
        }
    };
}

