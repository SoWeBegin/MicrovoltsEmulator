#include <iostream>
#include <chrono>
#include <format>
#include <asio/execution_context.hpp>
#include <boost/program_options.hpp>

#include "../include/MainServer.h"
#include "../include/Structures/AccountInfo/MainAccountInfo.h"
#include "../include/ConstantDatabase/CdbSingleton.h"

#include <ConstantDatabase/Structures/CdbWeaponsInfo.h>
#include <ConstantDatabase/Structures/CdbItemInfo.h>
#include <ConstantDatabase/Structures/CdbUpgradeInfo.h>
#include <ConstantDatabase/Structures/CdbCapsuleInfo.h>
#include <ConstantDatabase/Structures/CdbCapsulePackageInfo.h>
#include <ConstantDatabase/Structures/SetItemInfo.h>
#include <Utils/IPC_Structs.h>
#include <ConstantDatabase/Structures/CdbRewardInfo.h>


void printInitialInformation()
{
	auto const now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&now_time_t);
    auto time_s = std::put_time(&local_tm, "{:%Y-%m-%d %X}");
    std::cout << "[Info] Main server initialized on " << time_s << "\n\n";
}

int processCommandLine(int argc, char * argv[], uint16_t & port, std::string & cgd_path, std::string & db_path)
{
    try
    {
        boost::program_options::options_description desc("Main server usage", 1024, 512);
        desc.add_options()
                ("help",     "This help message")
                //("host,h",   boost::program_options::value<std::string>(&host)->required(),      "set the host server")
                ("port,p",   boost::program_options::value<uint16_t>(&port)->default_value(13005),             "The port MainServer will listen on. Default 13005")
                ("cgd", boost::program_options::value<std::string>(&cgd_path)->default_value("./Database/cgd_original/ENG"), "Path to the game (constant) databases folder. Default ./Database/cgd_original/ENG")
                ("database,db", boost::program_options::value<std::string>(&db_path)->default_value("./Database/GameDatabase.db"), "Path to the users databases. Default ./Database/GameDatabase.db")
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

void initializeCdbFiles(const std::string & cdbItemInfoPath)
{
	//const std::string cdbItemInfoPath = "../ExternalLibraries/cgd_original/ENG";
	const std::string cdbItemInfoName = "iteminfo.cdb";
	const std::string cdbSetItemInfoName = "setiteminfo.cdb";
	const std::string cdbWeaponItemInfoName = "itemweaponsinfo.cdb";
	const std::string cdbCapsuleInfoName = "gachaponinfo.cdb";
	const std::string cdbCapsulePackageInfoName = "gachaponpackageinfo.cdb";
	const std::string cdbUpgradeInfoName = "upgradeinfo.cdb";
	const std::string cdbRewardInfo = "rewardinfo.cdb";
	const std::string cdbGradeInfo = "gradeinfo.cdb";

	using cdbItems = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbItemInfo>;
	using setItems = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::SetItemInfo>;
	using cdbWeapons = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbWeaponInfo>;
	using upgradeInfos = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbUpgradeInfo>;
	using capsulePackageInfos = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbCapsulePackageInfo>;
	using capsuleInfos = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbCapsuleInfo>;
	using rewardInfo = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbRewardInfo>;
	using levelInfo = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbGradeInfo>;

	std::cout << "[Info] Initializing constant database maps...\n";
	cdbItems::initialize(cdbItemInfoPath, cdbItemInfoName);
	cdbItems::initializeItemTypes(cdbItemInfoPath, cdbWeaponItemInfoName, cdbItemInfoName);
	setItems::initialize(cdbItemInfoPath, cdbSetItemInfoName);
	cdbWeapons::initialize(cdbItemInfoPath, cdbWeaponItemInfoName);
	upgradeInfos::initialize(cdbItemInfoPath, cdbUpgradeInfoName);
	capsuleInfos::initialize(cdbItemInfoPath, cdbCapsuleInfoName);
	capsulePackageInfos::initialize(cdbItemInfoPath, cdbCapsulePackageInfoName);
	rewardInfo::initialize(cdbItemInfoPath, cdbRewardInfo);
	levelInfo::initialize(cdbItemInfoPath, cdbGradeInfo);
	std::cout << "[Info] Constant database successfully initialized.\n";
}

int main(int argc, char * argv[])
{
#if defined WIN32
    SetConsoleTitleW(L"Microvolts Main Server");
#endif
    asio::io_context io_context;

    uint16_t port;
    uint16_t server_id = 1;
    std::string cgd_path;
    std::string db_path;

    if(!processCommandLine(argc, argv, port, cgd_path, db_path))
        return 1;

    auto work_guard = asio::make_work_guard(io_context);
    Main::MainServer srv(io_context, db_path, port, server_id);

    initializeCdbFiles(cgd_path);
    Utils::IPCManager::cleanupSharedMemory();

    printInitialInformation();
    srv.asyncAccept();
    std::vector<std::thread> threads;
    const std::uint32_t num_threads = std::thread::hardware_concurrency();
    for (std::uint32_t i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&io_context]()
            {
                io_context.run();  
            });
    }

    io_context.run();
}