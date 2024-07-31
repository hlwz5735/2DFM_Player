set(target_name libsndfile)

add_library(${target_name} SHARED IMPORTED GLOBAL)

set_target_properties(${target_name} PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(${target_name} PROPERTIES
            IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/x64/sndfile.lib"
            IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/x64/sndfile.dll"
    )
endif()
