#include <iostream>
#include <chrono>
#include <format>
#include <asio/execution_context.hpp>
#include "include/AuthServer.h"

void printInitialInformation()
{
	auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
	auto const time_s = std::format("{:%Y-%m-%d %X}", time);
	std::cout << "Auth server initialized on " << time_s << "\n\n";
}

int main()
{
#ifdef WIN32
	SetConsoleTitleW(L"Microvolts Auth Server");
#endif
	asio::io_context io_context;
	Auth::AuthServer srv(io_context, 13000);

	printInitialInformation();

	srv.asyncAccept();
	io_context.run();
}