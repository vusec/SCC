cmake_minimum_required(VERSION 3.11)
project(scc)

set(CMAKE_CXX_STANDARD 17)


enable_testing()
include(cmake/scc.cmake)
add_subdirectory(extern)

add_subdirectory(scc)
