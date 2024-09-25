#include <iostream>
#include <chrono>
#include <string>
#include <thread>

#include <boost/program_options.hpp>

#include "../include/CastServer.h"
#include "../include/ConstantDatabase/CdbSingleton.h"
#include "../include/ConstantDatabase/Structures/CdbMapInfo.h"
#include "../include/Utils/HostSuicideUtils.h"

void printInitialInformation()
{
    auto const now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&now_time_t);
    auto time_s = std::put_time(&local_tm, "{:%Y-%m-%d %X}");
	std::cout << "[Info] Cast server initialized on " << time_s << "\n\n";
}

int processCommandLine(int argc, char * argv[], uint16_t & port, std::string & cgd_path)
{
	try
	{
		boost::program_options::options_description desc("Cast server usage", 1024, 512);
		desc.add_options()
		  ("help",     "This help message")
		  //("host,h",   boost::program_options::value<std::string>(&host)->required(),      "set the host server")
		  ("port,p",   boost::program_options::value<uint16_t>(&port)->default_value(13006),             "The port CastServer will listen on. Default 13006")
		  ("cgd", boost::program_options::value<std::string>(&cgd_path)->default_value("./Database/cgd_original/ENG"), "Path to the game (constant) databases folder. Default ./Database/cgd_original/ENG")
		;

		boost::program_options::variables_map vm;
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);

		if (vm.count("help"))
		{
			std::cout << desc << "\n";
			return false;
		}

		boost::program_options::notify(vm);
	}
	catch(boost::program_options::error& e)
	{
		std::cerr << "Error: " << e.what() << "\n";
		return false;
	}
	catch(...)
	{
		std::cerr << "Unknown error!" << "\n";
		return false;
	}

	return true;
}

int main(int argc, char * argv[])
{
#if defined WIN32
    SetConsoleTitleW(L"Microvolts Cast Server");
#endif
    asio::io_context io_context;

	uint16_t port;
	uint16_t server_id = 4;
	std::string cgd_path;

	if(!processCommandLine(argc, argv, port, cgd_path))
		return 1;

	std::cout << "[Info] Initializing constant database maps...\n";
	const std::string cdbMapInfoName = "mapinfo.cdb";
	using mapInfo = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbMapInfo>;
	mapInfo::initialize(cgd_path, cdbMapInfoName);

	Cast::Utils::initHeightDeaths();
	std::cout << "[Info] Constant database successfully initialized.\n";

    Cast::CastServer srv(io_context, port, server_id);

    printInitialInformation();
    srv.asyncAccept();

    auto workGuard = asio::make_work_guard(io_context);
    const std::uint32_t numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    for (std::uint32_t i = 0; i < numThreads; ++i)
    {
        threads.emplace_back([&io_context] 
            {
            io_context.run(); 
            });
    }

    for (unsigned int i = 0; i < numThreads; ++i)
        threads[i].join();
}

