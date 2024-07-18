cmake_minimum_required(VERSION 3.20)
project(2DFM_Player)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_utils/modules/")

add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(test)
