cmake_minimum_required(VERSION 3.26)
project(Microvolts-Emulator-V2)
enable_language(C CXX ASM)

#set(CMAKE_C_COMPILER gcc)
#set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)

include_directories(PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(PUBLIC ${CMAKE_CURRENT_LIST_DIR})
include_directories(PUBLIC ${CMAKE_SOURCE_DIR}/ExternalLibraries)

file(GLOB_RECURSE MAIN_SOURCES  ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
add_executable(MainServer ${MAIN_SOURCES})

target_link_libraries(MainServer
        Common
        SQLiteCpp
        sqlite3
)

if(WIN32)
    target_link_libraries(MainServer wsock32 ws2_32)
endif()

set_property(TARGET MainServer PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(MainServer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})