if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_subdirectory(libiconv)
endif ()

add_subdirectory(libsndfile)
