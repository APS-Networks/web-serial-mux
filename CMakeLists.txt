cmake_policy(SET CMP0048 NEW)

cmake_minimum_required(VERSION 3.16)
project(SerialMux)

add_subdirectory(cmake EXCLUDE_FROM_ALL)

add_subdirectory(bin)
add_subdirectory(lib)
add_subdirectory(scripts)
add_subdirectory(site)

