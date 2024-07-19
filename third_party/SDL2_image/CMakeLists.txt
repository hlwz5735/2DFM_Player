add_library(SDL2_image SHARED IMPORTED GLOBAL)

target_include_directories(SDL2_image INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

set_target_properties(SDL2_image PROPERTIES 
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/x64/SDL2_image.dll"
    IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/x64/SDL2_image.lib")
