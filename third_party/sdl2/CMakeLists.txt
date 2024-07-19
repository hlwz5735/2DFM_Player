add_library(sdl2 SHARED IMPORTED GLOBAL)

target_include_directories(sdl2 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

set_target_properties(sdl2 PROPERTIES 
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/x64/SDL2.dll"
    IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/x64/SDL2.lib")
