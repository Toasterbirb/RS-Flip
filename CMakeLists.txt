cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FLIP_GUI flip_gui)

project(rs-flip
	VERSION 0.1.0
	LANGUAGES CXX)

option(BIRB2D_GUI "Build Birb2D gui version" OFF)
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
include_directories(include/ vendor/)

add_executable(flip
	src/main.cpp
	src/flip/dailygoal.cpp
	src/flip/flips.cpp
	src/flip/margin.cpp
	src/flip/fliputils.cpp
	src/flip/avgstat.cpp
	src/flip/stats.cpp)

if (DEBUG)
	target_compile_options(flip PRIVATE -std=c++17 -g)
else()
	target_compile_options(flip PRIVATE -std=c++17)
endif()

target_precompile_headers(flip
    PRIVATE include/pch.hpp)

if (BIRB2D_GUI)
	include_directories(birb2d/include/)

	add_subdirectory(birb2d)
	include_directories(birb2d/vendor/SDL/include)

	add_executable(${FLIP_GUI}
		src/gui_main.cpp
		src/gui/guivars.cpp
		src/gui/tab.cpp)
	target_link_libraries(${FLIP_GUI} birb2d)

	# Copy the resources into the build directory
	file(COPY ./res DESTINATION ./)
	file(COPY ./birb2d/birb2d_res DESTINATION ./res/)
endif()
