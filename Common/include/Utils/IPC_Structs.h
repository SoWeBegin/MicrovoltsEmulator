#ifndef IPC_STRUCTS_H
#define IPC_STRUCTS_H

#include <cstdint>
#include <string>
#include <boost/interprocess/shared_memory_object.hpp> 
#include <boost/interprocess/mapped_region.hpp> 

namespace Utils
{
	// Main communicates MapId to Cast server.
	struct MapInfo // map_info
	{
		std::uint32_t mapId;
	};

	class IPCManager
	{
	public:
		template<typename T>
		static void ipc_mainToCast(const T& data, const std::string& roomIdentifier, const std::string& dataIdentifier)
		{
			try
			{
				using namespace boost::interprocess;
				std::string identifier = roomIdentifier + "_" + dataIdentifier;
				shared_memory_object shm(open_or_create, identifier.c_str(), read_write);
				shm.truncate(sizeof(T));
				mapped_region region(shm, read_write);
				std::memcpy(region.get_address(), &data, sizeof(T));
			}
			catch (boost::interprocess::interprocess_exception& e)
			{
				// std::cerr << "[Utils::ipc_mainToCast] exception: " << e.what() << '\n';
			}
		}

		template<typename T>
		static inline T ipc_castGetFromMain(const std::string& roomIdentifier, const std::string& dataIdentifier)
		{
			T data{};
			try
			{
				using namespace boost::interprocess;
				std::string identifier = roomIdentifier + "_" + dataIdentifier;
				shared_memory_object shm(open_only, identifier.c_str(), read_write);
				mapped_region region(shm, read_write);
				std::memcpy(&data, region.get_address(), sizeof(T));
				// this is intentional, reset the region
				std::memset(region.get_address(), 0, sizeof(T));
			}
			catch (boost::interprocess::interprocess_exception& e)
			{
				//std::cerr << "Error receiving data from shared memory: " << e.what() << '\n';
			}
			return data;
		}

		static void cleanupSharedMemory()
		{
			using namespace boost::interprocess;
			constexpr const std::uint16_t maxRooms = 300;
			std::string identifiers[] = { "_map_info" };

			for (std::uint16_t i = 1; i <= maxRooms; ++i)
			{
				for (const auto& suffix : identifiers)
				{
					const std::string identifier = std::to_string(i) + suffix;
					try
					{
						std::cout << "Cleaning up shared memory with ID: " << identifier << '\n';
						shared_memory_object::remove(identifier.c_str());
					}
					catch (const interprocess_exception& e)
					{
						std::cerr << "[cleanupSharedMemory] exception: " << e.what() << '\n';
					}
				}
			}
		}
	};
}

#endif