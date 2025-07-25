cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(FLIP_GUI flip_gui)

project(rs-flip
	VERSION 1.0.1
	LANGUAGES CXX)

option(DEBUG "Enable debug symbols" OFF)
option(FUZZ "Change input parsing to help with fuzzing" OFF)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif(CCACHE_FOUND)

if (FUZZ)
	add_definitions(-DFUZZING)
endif()

# Headers
include_directories(include/)

# Dependencies
add_subdirectory(vendor/clipp)
include_directories(vendor/clipp/include)

include_directories(vendor/doctest)

include_directories(vendor/json/include)


file(GLOB SRC ./src/*)
add_executable(flip ${SRC})
target_link_libraries(flip tbb)

# set(WARNINGS
# 	-pedantic -Wall -Wextra -Wcast-align
# 	-Wcast-qual -Wdisabled-optimization -Wformat=2
# 	-Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs
# 	-Wnoexcept -Woverloaded-virtual -Wsign-promo -Wstrict-null-sentinel -Werror -Wno-unused
# )
set(WARNINGS -pedantic -Wall -Wextra)

if (DEBUG)
	target_compile_options(flip PRIVATE -std=c++20 -g ${WARNINGS})
else()
	target_compile_options(flip PRIVATE -std=c++20 ${WARNINGS})
endif()

add_custom_target(test DEPENDS flip COMMAND flip test)
