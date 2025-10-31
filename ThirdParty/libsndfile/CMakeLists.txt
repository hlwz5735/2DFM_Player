set(target_name libsndfile_ext)

# 根据平台选择依赖管理方式
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Windows: 使用预编译的本地库文件
    add_library(${target_name} SHARED IMPORTED GLOBAL)
    set_target_properties(${target_name} PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
            IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/x64/sndfile.lib"
            IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/x64/sndfile.dll"
    )
    message(STATUS "Using precompiled libsndfile for Windows")
else()
    # Linux/macOS: 通过 pkg-config 查找系统安装的库
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SNDFILE REQUIRED sndfile)  # 移除 QUIET，使用 REQUIRED

    if (SNDFILE_FOUND)
        # 创建接口导入目标，而不是别名目标
        add_library(${target_name} INTERFACE IMPORTED GLOBAL)

        # 设置包含目录
        if(SNDFILE_INCLUDE_DIRS)
            target_include_directories(${target_name} INTERFACE ${SNDFILE_INCLUDE_DIRS})
        endif()

        # 设置编译选项
        if(SNDFILE_CFLAGS)
            target_compile_options(${target_name} INTERFACE ${SNDFILE_CFLAGS})
        endif()

        # 设置链接库
        if(SNDFILE_LIBRARIES)
            target_link_libraries(${target_name} INTERFACE ${SNDFILE_LIBRARIES})
        endif()

        # 设置链接目录
        if(SNDFILE_LIBRARY_DIRS)
            target_link_directories(${target_name} INTERFACE ${SNDFILE_LIBRARIES})
        endif()

        message(STATUS "Found libsndfile via pkg-config: ${SNDFILE_LIBRARIES}")
    else()
        message(FATAL_ERROR "libsndfile not found via pkg-config")
    endif()
endif()