

#include <iostream>
#include <chrono>
#include <format>
#include <asio/execution_context.hpp>
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
	auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
	auto const time_s = std::format("{:%Y-%m-%d %X}", time);
	std::cout << "[Info] Main server initialized on " << time_s << "\n\n";
}

void initializeCdbFiles()
{
	const std::string cdbItemInfoPath = "../ExternalLibraries/cgd_original/ENG";
	const std::string cdbItemInfoName = "iteminfo.cdb";
	const std::string cdbSetItemInfoName = "setiteminfo.cdb";
	const std::string cdbWeaponItemInfoName = "itemweaponsinfo.cdb";
	const std::string cdbCapsuleInfoName = "gachaponinfo.cdb";
	const std::string cdbCapsuleDisplayName = "gachapondisplay.cdb";
	const std::string cdbCapsulePackageInfoName = "packageinfo.cdb";
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
	using cdbCapsuleDisplay = Common::ConstantDatabase::CdbSingleton<Common::ConstantDatabase::CdbCapsuleDisplay>;

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
	cdbCapsuleDisplay::initialize(cdbItemInfoPath, cdbCapsuleDisplayName);
	std::cout << "[Info] Constant database successfully initialized.\n";
}

int main()
{
	SetConsoleTitleW(L"Microvolts Main Server");

	asio::io_context io_context;
	auto work_guard = asio::make_work_guard(io_context);

	Main::MainServer srv(io_context, 13005, 13004, 1);  

	printInitialInformation();
	initializeCdbFiles();
	Utils::IPCManager::cleanupSharedMemory();

	srv.asyncAccept();        
	srv.asyncAcceptAuthServer(); 

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
