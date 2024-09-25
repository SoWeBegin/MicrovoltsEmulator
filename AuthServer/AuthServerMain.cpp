#include <iostream>
#include <chrono>

#include <boost/program_options.hpp>
#include "include/AuthServer.h"

void printInitialInformation()  //TODO move this in common
{
    //auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
    //auto const time_s = std::format("{:%Y-%m-%d %X}", time);
    auto const now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&now_time_t);
    auto time_s = std::put_time(&local_tm, "{:%Y-%m-%d %X}");
    std::cout << "Auth server initialized on " << time_s << "\n\n";
}

//TODO this requires linking agains some boost shared lib, consider replacing with argparse
int processCommandLine(int argc, char * argv[], uint16_t & port, std::string & db_path)
{
	try
	{
		boost::program_options::options_description desc("Auth server usage", 1024, 512);
		desc.add_options()
		  ("help",     "This help message")
		  //("host,h",   boost::program_options::value<std::string>(&host)->required(),      "set the host server")
		  ("port,p",   boost::program_options::value<uint16_t>(&port)->default_value(13000),             "The port AuthServer will listen on. Default 13000")
		  ("database,db", boost::program_options::value<std::string>(&db_path)->default_value("./Database/GameDatabase.db"), "Path to the users database. Default ./Database/GameDatabase.db")
		;

		boost::program_options::variables_map vm;
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);

		if (vm.count("help"))
		{
			std::cout << desc << "\n";
			return false;
		}

		// There must be an easy way to handle the relationship between the
		// option "help" and "host"-"port"-"config"
		// Yes, the magic is putting the po::notify after "help" option check
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
#ifdef WIN32
	SetConsoleTitleW(L"Microvolts Auth Server");
#endif

	uint16_t port;
	std::string db_path;

	if(!processCommandLine(argc, argv, port, db_path))
		return 1;

	asio::io_context io_context;
	Auth::AuthServer srv(io_context, db_path, port);

	printInitialInformation();

	srv.asyncAccept();
	io_context.run();
}