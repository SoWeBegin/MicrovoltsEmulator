cmake_minimum_required(VERSION 3.26)
project(Microvolts-Emulator-V2)
enable_language(C CXX ASM)

#set(CMAKE_C_COMPILER gcc)
#set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)

#SET(CMAKE_EXE_LINKER_FLAGS  "-Wl,--allow-multiple-definition")

#boost
find_package(Boost 1.85.0 COMPONENTS system)
IF(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
ELSE ()
    #set(Boost_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/ExternalLibraries/boost_1_85_0")
    set(Boost_INCLUDE_DIRS "D:/Programming/boost_1_85_0")
endif()

IF (WIN32)

    set(CMAKE_CXX_FLAGS "-D_WIN32_WINNT=0x0601 -Wa,-mbig-obj")
ELSE()
    set(CMAKE_EXECUTABLE_SUFFIX ".elf")
    include_directories(PUBLIC ${CMAKE_SOURCE_DIR}/ExternalLibraries/sal)
ENDIF()

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)


include_directories(PUBLIC ${Boost_INCLUDE_DIRS})
include_directories(PUBLIC ${CMAKE_SOURCE_DIR}/ExternalLibraries)

# COMMON LIBRARY
include_directories(PUBLIC ${CMAKE_SOURCE_DIR}/ExternalLibraries/asio-1.30.2/include)
add_subdirectory(${CMAKE_SOURCE_DIR}/SQLiteCpp)

add_subdirectory(Common)
include_directories(${CMAKE_CURRENT_LIST_DIR}/Common/include)

# AUTH SERVER
add_subdirectory(AuthServer)

# CAST SERVER
add_subdirectory(CastServer)

# MAIN SERVER
add_subdirectory(MainServer)