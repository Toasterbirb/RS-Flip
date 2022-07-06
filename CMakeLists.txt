cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(rs-flip
	VERSION 0.0.1
	LANGUAGES CXX)

option(TESTS "Enable tests in the build" OFF)

if (NOT TESTS)
	add_definitions(-DDOCTEST_CONFIG_DISABLE)
endif()

# Headers
include_directories(include/ vendor/)

add_executable(rs-flip
	src/main.cpp
	src/flips.cpp
	src/margin.cpp
	src/utils.cpp)

target_compile_options(rs-flip PRIVATE -std=c++17)
#target_link_libraries(rs-flip)
