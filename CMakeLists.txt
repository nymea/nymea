cmake_minimum_required(VERSION 2.8.9)

project(Hive C CXX)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-permissive -pedantic -Wall -Wextra -fPIC")

include(FindPkgConfig)

find_package(Qt5Core)

add_subdirectory(libhive)
add_subdirectory(server)

