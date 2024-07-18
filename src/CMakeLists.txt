set(target_name 2DFM_Player)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS})

include(base/CMakeLists.txt)
include(2dfm/CMakeLists.txt)
include(math/CMakeLists.txt)
aux_source_directory(. sources)
add_executable(${target_name} ${sources} ${math_sources} ${2dfm_sources} ${base_sources})

target_link_libraries(${target_name}
        ${SDL2_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES}
        rapidjson)
