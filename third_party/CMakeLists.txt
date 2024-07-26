add_subdirectory(rapidjson)
add_subdirectory(GLEW)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_subdirectory(libiconv)
    add_subdirectory(sdl2)
    add_subdirectory(SDL2_image)
    add_subdirectory(SDL2_mixer)
endif ()
