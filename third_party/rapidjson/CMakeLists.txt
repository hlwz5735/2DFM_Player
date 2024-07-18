project(rapidjson)

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE ./include)
set_target_properties(rapidjson PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include")
