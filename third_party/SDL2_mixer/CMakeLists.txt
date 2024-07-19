add_library(SDL2_mixer SHARED IMPORTED GLOBAL)

target_include_directories(SDL2_mixer INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

set_target_properties(SDL2_mixer PROPERTIES 
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/x64/SDL2_mixer.dll"
    IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/x64/SDL2_mixer.lib")
