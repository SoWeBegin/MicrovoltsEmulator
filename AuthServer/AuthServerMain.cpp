#include <iostream>
#include <chrono>
#include <format>
#include <asio/execution_context.hpp>
#include "include/AuthServer.h"

void printInitialInformation()
{
    //auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
    //auto const time_s = std::format("{:%Y-%m-%d %X}", time);
    auto const now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&now_time_t);
    auto time_s = std::put_time(&local_tm, "{:%Y-%m-%d %X}");
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