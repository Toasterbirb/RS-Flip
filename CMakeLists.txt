cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FLIP_GUI flip_gui)

project(rs-flip
	VERSION 1.0.1
	LANGUAGES CXX)

option(TESTS "Enable tests in the build" OFF)
option(DEBUG "Enable debug symbols" OFF)

if (NOT TESTS)
	add_definitions(-DDOCTEST_CONFIG_DISABLE)
endif()

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif(CCACHE_FOUND)

# Headers
include_directories(include/)

add_executable(flip
	src/main.cpp
	src/dailygoal.cpp
	src/flips.cpp
	src/margin.cpp
	src/fliputils.cpp
	src/avgstat.cpp
	src/stats.cpp
	src/table.cpp
)

if (DEBUG)
	target_compile_options(flip PRIVATE -std=c++20 -g)
else()
	target_compile_options(flip PRIVATE -std=c++20)
endif()

target_precompile_headers(flip
    PRIVATE include/pch.hpp)
