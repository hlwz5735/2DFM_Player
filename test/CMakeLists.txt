add_executable(test01
        test01.cpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    find_package(Iconv REQUIRED)
endif()

target_link_libraries(test01 PUBLIC Iconv)

# target_include_directories(test01 PUBLIC ../third_party/libiconv/include)
