cmake_minimum_required(VERSION 3.26)
project(Microvolts-Emulator-V2)
enable_language(C CXX ASM)

#set(CMAKE_C_COMPILER gcc)
#set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)

include_directories(PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(PUBLIC ${CMAKE_CURRENT_LIST_DIR})
include_directories(PUBLIC ${CMAKE_SOURCE_DIR}/ExternalLibraries/asio-1.24.0/include)

file(GLOB_RECURSE LIBRARY_SOURCES  ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
add_library(Common SHARED
        Common.cpp
        ${LIBRARY_SOURCES}
)

if(WIN32)
    target_link_libraries(Common wsock32 ws2_32)
endif()

set_target_properties(Common PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

set_property(TARGET Common PROPERTY POSITION_INDEPENDENT_CODE ON)