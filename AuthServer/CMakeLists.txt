cmake_minimum_required(VERSION 3.26)
project(Microvolts-Emulator-V2)
enable_language(C CXX ASM)

set(CMAKE_CXX_STANDARD 20)

include_directories(PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(PUBLIC ${CMAKE_CURRENT_LIST_DIR})


add_executable(AuthServer
        AuthServerMain.cpp
        src/AuthServer.cpp
        src/Network/AuthSession.cpp
        src/Database/DbPlayerInfo.cpp
)

target_link_libraries(AuthServer
        Common
        SQLiteCpp
        sqlite3
        ${Boost_LIBRARIES}
)

if(WIN32)
    target_link_libraries(AuthServer wsock32 ws2_32)
endif()

set_property(TARGET AuthServer PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(AuthServer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})