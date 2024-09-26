#ifndef CAPSULE_MANAGER_H
#define CAPSULE_MANAGER_H
#include <cstdint>
#include "../Persistence/MainScheduler.h"

namespace Main
{
	namespace Classes
	{
		class CapsuleManager
		{
		private:
			uint32_t m_jackpot;
			Main::Persistence::MainScheduler& m_scheduler;
			Main::Persistence::PersistentDatabase& database;

		public:
			explicit CapsuleManager(Main::Persistence::MainScheduler& m_scheduler, Main::Persistence::PersistentDatabase& database);

			~CapsuleManager();

			std::uint32_t getJackpot();
			void addJackpot(uint32_t value);
			void subJackpot(uint32_t value);
			void setJackpot(uint32_t jackpot);
		};
	}
}

#endif
