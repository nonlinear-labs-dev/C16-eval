cmake_minimum_required(VERSION 3.0)

message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

project(lpc-tool)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra")

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(lpc-tool ${SOURCES})

target_include_directories(lpc-tool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/../../firmware/src/main/src)

target_link_libraries(lpc-tool PRIVATE asound m ${CURSES_LIBRARY})
