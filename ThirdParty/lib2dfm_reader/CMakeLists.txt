cmake_minimum_required(VERSION 3.20)
project(lib2dfm_reader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 源文件
file(GLOB_RECURSE LIB_SOURCES src/*.cpp)

# 构建为动态库
add_library(lib2dfm_reader SHARED ${LIB_SOURCES})

target_compile_definitions(lib2dfm_reader PRIVATE LIB2DFM_READER_BUILD)

# MSVC: 源文件为 UTF-8 编码
if(MSVC)
    target_compile_options(lib2dfm_reader PRIVATE /utf-8)
endif()

target_include_directories(lib2dfm_reader
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Windows 下导出所有符号或依赖 DFM_API 宏
if(WIN32)
    set_target_properties(lib2dfm_reader PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS OFF
        PREFIX ""
        OUTPUT_NAME "2dfm_reader"
    )
else()
    set_target_properties(lib2dfm_reader PROPERTIES
        OUTPUT_NAME "2dfm_reader"
        # 隐藏内部符号
        CXX_VISIBILITY_PRESET hidden
        C_VISIBILITY_PRESET hidden
    )
endif()
