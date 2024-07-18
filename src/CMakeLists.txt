set(target_name 2DFM_Player)

#find_package(SDL2 REQUIRED)
#find_package(SDL2_image REQUIRED)
#find_package(SDL2_mixer REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS})

include(base/CMakeLists.txt)
include(2dfm/CMakeLists.txt)
include(math/CMakeLists.txt)
aux_source_directory(. sources)
add_executable(${target_name} ${sources} ${math_sources} ${2dfm_sources} ${base_sources})

message("hahaha - ${SDL2_LIBRARIES}")
target_link_libraries(${target_name}
        sdl2
        ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES}
        rapidjson)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/sdl2/lib/win32/x64/SDL2.dll" $<TARGET_FILE_DIR:2DFM_Player>
)
endif()

