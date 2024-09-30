cmake_minimum_required(VERSION 3.26)
project(Microvolts-Emulator-V2)
enable_language(C CXX ASM)

set(CMAKE_CXX_STANDARD 20)

include_directories(PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(PUBLIC ${CMAKE_CURRENT_LIST_DIR})
include_directories(PUBLIC ${CMAKE_SOURCE_DIR}/ExternalLibraries/DirectXMath/Inc)

file(GLOB_RECURSE CAST_SOURCES  ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
add_executable(Cast ${CAST_SOURCES})

target_link_libraries(Cast ${Boost_LIBRARIES} Common)

set_property(TARGET Cast PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(Cast PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})