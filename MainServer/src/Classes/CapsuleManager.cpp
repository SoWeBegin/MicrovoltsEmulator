#include "../../include/Classes/CapsuleManager.h"

namespace Main
{
	namespace Classes
	{
		CapsuleManager::CapsuleManager(Main::Persistence::MainScheduler& m_scheduler, Main::Persistence::PersistentDatabase& database)
			: m_scheduler{ m_scheduler }
			, database{ database }
		{
		}

		CapsuleManager::~CapsuleManager()
		{
		}

		std::uint32_t CapsuleManager::getJackpot()
		{
			m_jackpot = database.getCapsuleJackpot();
			return m_jackpot;
		}

		void CapsuleManager::addJackpot(std::uint32_t value)
		{
			setJackpot(m_jackpot + value);
		}

		void CapsuleManager::subJackpot(std::uint32_t value)
		{
			setJackpot(m_jackpot - value);
		}

		void CapsuleManager::setJackpot(std::uint32_t value)
		{
			m_jackpot = value < 300000 ? 300000 : value;
			m_scheduler.immediatePersist(0, &Main::Persistence::PersistentDatabase::updateCapsuleJackpot, m_jackpot);
		}
	}
}
