#include <iostream>
#include <chrono>
#include <string>
#include <format>
#include <asio/execution_context.hpp>
#include "../include/CastServer.h"
#include "../include/ConstantDatabase/CdbSingleton.h"
#include "../include/ConstantDatabase/Structures/CdbMapInfo.h"
#include "../include/Utils/HostSuicideUtils.h"

void printInitialInformation()
{
	auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
	auto const time_s = std::format("{:%Y-%m-%d %X}", time);
	std::cout << "[Info] Cast server initialized on " << time_s << "\n\n";

	std::cout << "[Info] Initializing constant database maps...\n";
	const std::string cdbItemInfoPath = "../ExternalLibraries/cgd_original/ENG";
	const std::string cdbMapInfoName = "mapinfo.cdb";
	using mapInfo = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbMapInfo>;
	mapInfo::initialize(cdbItemInfoPath, cdbMapInfoName);

	Cast::Utils::initHeightDeaths();
	std::cout << "[Info] Constant database successfully initialized.\n";
}



int main()
{
    SetConsoleTitleW(L"Microvolts Cast Server");

    asio::io_context io_context;

    Cast::CastServer srv(io_context, 13006, 4);

    printInitialInformation();
    srv.asyncAccept();

    auto workGuard = asio::make_work_guard(io_context);
    const std::uint32_t numThreads = std::thread::hardware_concurrency();
    std::vector<std::jthread> threads;
    for (std::uint32_t i = 0; i < numThreads; ++i)
    {
        threads.emplace_back([&io_context] 
            {
            io_context.run(); 
            });
    }
}

