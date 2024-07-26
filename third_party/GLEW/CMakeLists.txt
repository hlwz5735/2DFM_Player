project(glew)
add_library(glew SHARED IMPORTED GLOBAL)
set_target_properties(glew PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(glew PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/x64/glew32.dll"
        IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/x64/glew32.lib")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set_target_properties(glew PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/mac/libGLEW.2.1.dylib")
endif()
