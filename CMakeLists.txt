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

set(WARNINGS
	-pedantic -Wall -Wextra -Wcast-align
	-Wcast-qual -Wdisabled-optimization -Wformat=2
	-Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs
	-Wnoexcept -Woverloaded-virtual -Wsign-promo -Wstrict-null-sentinel -Werror -Wno-unused
)

if (DEBUG)
	target_compile_options(flip PRIVATE -std=c++20 -g ${WARNINGS})
else()
	target_compile_options(flip PRIVATE -std=c++20 ${WARNINGS})
endif()

target_precompile_headers(flip
    PRIVATE include/pch.hpp)
