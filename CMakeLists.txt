cmake_minimum_required (VERSION 3.25)
project (controlService)

# Set the C++ standard to C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")

# Force WebSocket++ to use 100% C++11 mode, so that it doesn't try to look for Boost
# (Note that under MinGW, Boost.Thread is still required, due to a bug in MinGW that prevents the C++11 version from being used)
add_definitions(-D_WEBSOCKETPP_CPP11_STRICT_)

# Add the third-party headers directory to the search path
include_directories(SYSTEM "${PROJECT_SOURCE_DIR}/external" "${PROJECT_SOURCE_DIR}/include" )

# Compile jsoncpp from source
add_library(jsoncpp STATIC ${PROJECT_SOURCE_DIR}/external/jsoncpp.cpp)

aux_source_directory(./src DIR_SRCS)

add_executable(controlService ${DIR_SRCS})
target_link_libraries (controlService jsoncpp)