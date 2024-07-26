set(target_name 2DFM_Player)

set(SDL2_LIB_NAME "SDL2")
set(SDL2_IMAGE_NAME "SDL2_image")
set(SDL2_MIXER_NAME "SDL2_mixer")
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS})

    set(SDL2_LIB_NAME ${SDL2_LIBRARIES})
    set(SDL2_IMAGE_NAME ${SDL2_IMAGE_LIBRARIES})
    set(SDL2_MIXER_NAME ${SDL2_MIXER_LIBRARIES})
endif()

aux_source_directory(. sources)

include(base/CMakeLists.txt)
include(2dfm/CMakeLists.txt)
include(math/CMakeLists.txt)
include(game/CMakeLists.txt)

message(${2dfm_sources})

add_executable(${target_name}
    ${sources}
    ${math_sources}
    ${2dfm_sources}
    ${base_sources}
    ${game_sources}
)

target_link_libraries(${target_name}
        ${SDL2_LIB_NAME}
        ${SDL2_IMAGE_NAME}
        ${SDL2_MIXER_NAME}
        Iconv
        rapidjson)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/sdl2/lib/windows/x64/SDL2.dll" 
            "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/SDL2_image/lib/windows/x64/SDL2_image.dll" 
            "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/SDL2_mixer/lib/windows/x64/SDL2_mixer.dll" 
            $<TARGET_FILE_DIR:2DFM_Player>
)
endif()
