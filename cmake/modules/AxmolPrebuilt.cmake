# ============================================================================
# axmol 引擎「预编译树」支持（跨平台，纯 CMake，无需任何平台脚本）
#
# 目的：引擎只编译一次，之后本工程每次构建只编译自己的代码。
#
# 机制：让 AX_PREBUILT_DIR 指向一棵“已独立配置并构建过的 axmol 工程树”
#       （路径相对 AX_ROOT）。本工程 CMakeLists 里的 _AX_USE_PREBUILT 判断会
#       自动生效：命中则跳过 add_subdirectory(${_AX_ROOT}/core)，改用 axmol 官方的
#       ax_link_cxx_prebuilt() 链接引擎库。
#
# 平台：axmol 官方只提供 Windows / Linux 的预编译通道；
#       macOS / iOS / tvOS / Android / WASM 会自动回退到源码编译，
#       这里不做特殊处理，也不需要平台专用脚本。
#
# 使用流程（三步，命令跨平台一致）：
#   1) 正常配置本工程（此时引擎仍从源码编译，配置日志会给出下一步命令）
#   2) 一次性生成预编译树：构建目标 axmol_engine_prebuilt
#      CLI: cmake --build <本工程构建目录> --target axmol_engine_prebuilt
#      CLion: 目标列表里直接构建 axmol_engine_prebuilt
#   3) 重新配置 CMake（CLion: Reload CMake Project）——此时自动检测到预编译树，
#      引擎不再参与本工程的编译。删除预编译树目录即回退到源码编译。
#
# 注意事项：
#   * 预编译树必须用「与本工程相同的生成器/平台/工具集/配置」构建，否则目录布局与
#     axmol 链接助手的预期不一致
#     （Ninja 单配置：lib/ 与 bin/ 平铺；VS/Xcode 多配置：lib/$(Configuration) 等）。
#     本文件的目标已自动沿用本工程的生成器/平台/工具集/配置。
#   * 预编译树里的引擎选项由那棵树自己的 CMakeCache 决定；目标会把
#     AxmolEngineOptions.cmake 里的选项原样转发，保证两种模式引擎配置一致。
#     改了引擎选项要重新构建预编译树。
#   * 链接方式由选项 AXMOL_ENGINE_SHARED 决定（默认静态）：
#       静态 prebuilt/<os>-<arch>-<cfg>         —— 单个 exe；链接要过一遍庞大的引擎 .lib
#       动态 prebuilt/<os>-<arch>-<cfg>-shared  —— 只链接导入库，引擎以 DLL/.so 复用，日常链接显著变快
#     两棵树可并存，切换只需重新配置；动态模式下本工程会自动以共享方式链接
#     （axmol 的助手据此定义 AX_DLLIMPORT，并在 Windows 上把引擎 DLL 拷到输出目录）。
#     实际生效的链接方式以「预编译树自己的缓存」为准，指向哪种树就按哪种链接，不会错配。
#   * 预编译树落在引擎目录内部（AX_PREBUILT_DIR 相对 AX_ROOT），
#     清理引擎检出目录（如 axmol 的清理/更新命令）会把它一并删掉，重新生成即可。
#   * 注意 axmol 内部有一处用 `DEFINED AX_PREBUILT_DIR`（而非“目录是否存在”）
#     来切换资源同步策略，因此本文件只在预编译树真正存在时才定义该变量，
#     源码模式下保持“未定义”，与官方语义一致。
# ============================================================================

# ============================================================================
# 小工具：把 CMake 里各种布尔写法归一化（缓存里可能是 ON/OFF，也可能是 TRUE/FALSE）
#   由下面的"预编译树体检"使用。
# ============================================================================
function(axmol_normalize_bool out_var value)
    string(TOUPPER "${value}" _axmol_norm_value)
    if(_axmol_norm_value STREQUAL "ON" OR _axmol_norm_value STREQUAL "TRUE"
       OR _axmol_norm_value STREQUAL "YES" OR _axmol_norm_value STREQUAL "1")
        set(${out_var} TRUE PARENT_SCOPE)
    elseif(_axmol_norm_value STREQUAL "OFF" OR _axmol_norm_value STREQUAL "FALSE"
           OR _axmol_norm_value STREQUAL "NO" OR _axmol_norm_value STREQUAL "0"
           OR _axmol_norm_value STREQUAL "")
        set(${out_var} FALSE PARENT_SCOPE)
    else()
        set(${out_var} "${value}" PARENT_SCOPE)
    endif()
endfunction()

# ============================================================================
# 供下面延迟调用：把共享引擎的运行库补齐到 app 输出目录
#   背景：axmol 的 ax_link_cxx_prebuilt 会拷贝 axmol/GUI/… 等 DLL，但漏了 fmt（fmtd.dll），
#   而 axmol.dll 自身就依赖 fmtd.dll —— 缺了它程序会启动失败（0xc0000135）。
#   这里不逐个点名，直接把预编译树 bin 下的所有 DLL 都补上（copy_if_different，重复拷贝是空操作），
#   以后引擎新增独立 DLL 也能自动覆盖。
#   本文件在 add_executable 之前被包含，所以用 cmake_language(DEFER) 延迟到目录作用域末尾执行。
# ============================================================================
function(axmol_copy_engine_runtime_dlls app_name)
    if(NOT TARGET ${app_name})
        message(STATUS "axmol 引擎：跳过运行库补齐（目标 ${app_name} 不存在）")
        return()
    endif()
    add_custom_command(TARGET ${app_name} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${ARGN} "$<TARGET_FILE_DIR:${app_name}>"
        COMMENT "补齐共享版 axmol 引擎运行库"
        COMMAND_EXPAND_LISTS
        VERBATIM)
endfunction()

# ============================================================================
# 供下面延迟调用：修正 app 侧的 DLL 宏（共享模式）
#   本工程的 CMakeLists 在预编译分支里调用了 use_ax_compile_define(${APP_NAME})，
#   而该函数是给「会被编进引擎的目标」准备的（AXConfigDefine.cmake）：
#       if(BUILD_SHARED_LIBS)
#         target_compile_definitions(${target} PRIVATE AX_DLLEXPORT INTERFACE AX_DLLIMPORT)
#       endif()
#   于是 app 同时拿到 AX_DLLEXPORT(PRIVATE) 与 AX_DLLIMPORT(=1，由 ax_link_cxx_prebuilt 添加)。
#   而 core/platform/PlatformDefine.h 的判定是：
#       #if defined(AX_DLLEXPORT)      -> __declspec(dllexport)
#       #elif defined(AX_DLLIMPORT)    -> __declspec(dllimport)
#   EXPORT 优先 → app 把 axmol 的静态数据成员当成「由我自己导出」→ 生成直接引用，
#   而数据符号在导入库里没有同名跳转桩（函数有）→ 链接期 LNK2001：
#   Vec2::ZERO / Vec2::ANCHOR_MIDDLE / Rect::ZERO / Color3B::WHITE / Color4F::BLACK /
#   BlendFunc::ADDITIVE / FileUtils::s_exeDir / AudioEngine::_maxInstances / Image::PNG_* 等。
#   app 是纯粹的导入方，只应保留 AX_DLLIMPORT，所以这里把 AX_DLLEXPORT 去掉。
# ============================================================================
function(axmol_fix_app_shared_macros app_name)
    if(NOT TARGET ${app_name})
        return()
    endif()
    get_target_property(_axmol_app_defs ${app_name} COMPILE_DEFINITIONS)
    if(NOT _axmol_app_defs)
        return()
    endif()
    set(_axmol_app_defs_before "${_axmol_app_defs}")
    list(REMOVE_ITEM _axmol_app_defs AX_DLLEXPORT -DAX_DLLEXPORT)
    if(NOT "${_axmol_app_defs}" STREQUAL "${_axmol_app_defs_before}")
        set_target_properties(${app_name} PROPERTIES COMPILE_DEFINITIONS "${_axmol_app_defs}")
        message(STATUS "axmol 引擎：共享模式下已从 ${app_name} 移除 AX_DLLEXPORT"
                       "（app 只能以“导入方”身份编译 axmol 头，否则静态数据成员会被当作自行导出而链接失败）")
    endif()
endfunction()

# ── 链接方式：静态库 / 动态库 ────────────────────────────────────────────────
# 静态：app 每次链接都要过一遍庞大的引擎 .lib（Debug 下 axmol.lib 约 250MB 量级）
# 动态：app 只链接很小的导入库，引擎以 DLL/.so 形式复用 → 日常链接显著变快
#      代价：需要随包分发引擎 DLL；Debug/Release 的库不能混用
# 打开本选项后，预编译树的默认目录会带 -shared 后缀（与已有静态树并存，互不干扰）
option(AXMOL_ENGINE_SHARED "引擎以动态库(DLL/.so)方式链接，加快日常链接速度（需重新生成预编译树）" OFF)

# ── 当前平台是否支持预编译引擎 ────────────────────────────────────────────────
set(_axmol_prebuilt_os "")
if(WIN32)
    set(_axmol_prebuilt_os "win")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(_axmol_prebuilt_os "linux")
endif()

if(_axmol_prebuilt_os)
    # 架构：多配置生成器（VS 等）以 GENERATOR_PLATFORM 为准，其余按指针宽度推断
    if(CMAKE_GENERATOR_PLATFORM)
        string(TOLOWER "${CMAKE_GENERATOR_PLATFORM}" _axmol_prebuilt_arch)
        if(_axmol_prebuilt_arch STREQUAL "win32")
            set(_axmol_prebuilt_arch "x86")
        endif()
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_axmol_prebuilt_arch "x64")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(_axmol_prebuilt_arch "x86")
    else()
        set(_axmol_prebuilt_arch "unknown")
    endif()

    # 配置：单配置生成器（Ninja / Makefiles）的库是平铺的，一套配置一棵树；
    #       多配置生成器（VS / Xcode）一棵树同时供 Debug / Release 使用
    if(CMAKE_CONFIGURATION_TYPES)
        set(_axmol_prebuilt_cfg "multi")
    else()
        string(TOLOWER "${CMAKE_BUILD_TYPE}" _axmol_prebuilt_cfg)
        if(_axmol_prebuilt_cfg STREQUAL "")
            set(_axmol_prebuilt_cfg "debug")
        endif()
    endif()

    # 默认位置：prebuilt/<os>-<arch>-<cfg>[-shared]
    #   静态用不带后缀的名字（保持与既有预编译树兼容），动态加 -shared 后缀，两棵树可并存
    if(AXMOL_ENGINE_SHARED)
        set(_axmol_prebuilt_link_suffix "-shared")
    else()
        set(_axmol_prebuilt_link_suffix "")
    endif()

    # 使用者可用 AXMOL_ENGINE_PREBUILT_DIR 显式覆盖位置；留空则按上面规则自动推导
    set(AXMOL_ENGINE_PREBUILT_DIR "" CACHE STRING
        "覆盖引擎预编译树位置（相对 AX_ROOT）；留空表示自动推导为 prebuilt/<os>-<arch>-<cfg>[-shared]")
    if(NOT "${AXMOL_ENGINE_PREBUILT_DIR}" STREQUAL "")
        set(_axmol_prebuilt_default "${AXMOL_ENGINE_PREBUILT_DIR}")
    else()
        set(_axmol_prebuilt_default
            "prebuilt/${_axmol_prebuilt_os}-${_axmol_prebuilt_arch}-${_axmol_prebuilt_cfg}${_axmol_prebuilt_link_suffix}")
    endif()
endif()

# ── 启用判定 ──────────────────────────────────────────────────────────────────
# 实际参与 axmol 判定的是 AX_PREBUILT_DIR，两种来源：
#   1) 使用者显式指定（-DAX_PREBUILT_DIR=... / preset / 上层工程）
#   2) 默认位置下确实存在预编译树 —— 此时本文件替使用者填上
set(_axmol_engine_tree "")
if(_axmol_prebuilt_os AND NOT _is_axmol_embed)
    if(DEFINED AX_PREBUILT_DIR AND NOT "${AX_PREBUILT_DIR}" STREQUAL "")
        set(_axmol_engine_tree "${AX_PREBUILT_DIR}")
    elseif(IS_DIRECTORY "${_AX_ROOT}/${_axmol_prebuilt_default}")
        set(AX_PREBUILT_DIR "${_axmol_prebuilt_default}")
        set(_axmol_engine_tree "${AX_PREBUILT_DIR}")
    else()
        set(_axmol_engine_tree "${_axmol_prebuilt_default}")
    endif()

    if(DEFINED AX_PREBUILT_DIR AND IS_DIRECTORY "${_AX_ROOT}/${AX_PREBUILT_DIR}")
        # 补齐引擎运行库（见文件上方 axmol_copy_engine_runtime_dlls 的说明）
        if(AXMOL_ENGINE_SHARED)
            set(_axmol_bin_dir "${_AX_ROOT}/${AX_PREBUILT_DIR}/bin")
            if(CMAKE_CONFIGURATION_TYPES)
                # 多配置生成器：DLL 在 bin/<Config>/ 下，按配置加守卫，避免串配置拷贝
                foreach(_axmol_cfg IN LISTS CMAKE_CONFIGURATION_TYPES)
                    file(GLOB _axmol_cfg_dlls "${_axmol_bin_dir}/${_axmol_cfg}/*.dll")
                    if(_axmol_cfg_dlls)
                        list(LENGTH _axmol_cfg_dlls _axmol_cfg_dll_count)
                        message(STATUS "axmol 引擎：共享模式将补齐 ${_axmol_cfg_dll_count} 个引擎运行库"
                                       "（配置 ${_axmol_cfg}）")
                        cmake_language(DEFER CALL axmol_copy_engine_runtime_dlls "${APP_NAME}"
                                       "$<$<CONFIG:${_axmol_cfg}>:${_axmol_cfg_dlls}>")
                    endif()
                endforeach()
            else()
                file(GLOB _axmol_tree_dlls "${_axmol_bin_dir}/*.dll")
                if(_axmol_tree_dlls)
                    list(LENGTH _axmol_tree_dlls _axmol_tree_dll_count)
                    message(STATUS "axmol 引擎：共享模式将补齐 ${_axmol_tree_dll_count} 个引擎运行库到输出目录")
                    cmake_language(DEFER CALL axmol_copy_engine_runtime_dlls "${APP_NAME}" "${_axmol_tree_dlls}")
                endif()
            endif()
            # app 侧的 DLL 宏也要在目录作用域末尾修正（见 axmol_fix_app_shared_macros 的说明）
            cmake_language(DEFER CALL axmol_fix_app_shared_macros "${APP_NAME}")
        endif()

        # 共享模式下的 GLFW 导入库命名不一致：
        #   GLFW 自己为了避冲突会把 MSVC 共享构建的导入库改名
        #   （3rdparty/glfw/src/CMakeLists.txt: IMPORT_SUFFIX "dll.lib" → glfwdll.lib，
        #     而 DLL 名依旧是 glfw.dll）；
        #   但 axmol 的 prebuilt 链接助手（AXLinkHelpers.cmake）里写的是裸名 glfw，
        #   CMake 会按默认规则去找 glfw.lib → 链接期 LNK1104: 无法打开文件"glfw.lib"。
        # 这里在预编译树里补一个同名副本：导入库内容等价，其引用的 DLL 仍是 glfw.dll。
        if(AXMOL_ENGINE_SHARED)
            set(_axmol_glfw_link "${_AX_ROOT}/${AX_PREBUILT_DIR}/lib/glfw.lib")
            set(_axmol_glfw_dll_link "${_AX_ROOT}/${AX_PREBUILT_DIR}/lib/glfwdll.lib")
            if(NOT EXISTS "${_axmol_glfw_link}" AND EXISTS "${_axmol_glfw_dll_link}")
                file(COPY_FILE "${_axmol_glfw_dll_link}" "${_axmol_glfw_link}")
                message(STATUS "axmol 引擎：已在预编译树补建 glfw.lib（共享构建的导入库实际名为 glfwdll.lib，"
                               "而链接助手按 glfw.lib 取名）")
            endif()
        endif()

        # 自定义着色器的 axslcc 参数（--lang/--profile）取决于引擎侧定义的缓存变量：
        # core/CMakeLists.txt 里的 set(AX_GLES_PROFILE ... CACHE ...)，以及 AX_USE_GL / AX_USE_METAL。
        # 源码模式下它们随引擎子工程一起被建立；预编译模式下引擎不参与本工程配置，
        # 这些值要等 ax_link_cxx_prebuilt() 里的 load_cache 才载入 —— 而本工程的自定义着色器
        # （ax_setup_app_config 阶段）在那之前就编译了，于是 axslcc 缺少 --lang，
        # 报 "Shader language is not specified"。
        # 这里提前从引擎树缓存取值，保证自定义着色器能按与引擎一致的 profile 编译。
        set(_axmol_tree_cache "${_AX_ROOT}/${AX_PREBUILT_DIR}/CMakeCache.txt")
        if(EXISTS "${_axmol_tree_cache}")
            foreach(_axmol_var IN ITEMS AX_GLES_PROFILE AX_USE_GL AX_USE_METAL)
                file(STRINGS "${_axmol_tree_cache}" _axmol_var_lines REGEX "^${_axmol_var}:[^=]*=")
                if(_axmol_var_lines)
                    list(GET _axmol_var_lines 0 _axmol_var_line)
                    string(REGEX REPLACE "^[^=]*=" "" _axmol_var_value "${_axmol_var_line}")
                    set(${_axmol_var} "${_axmol_var_value}")
                endif()
            endforeach()
            unset(_axmol_var_lines)
            unset(_axmol_var_line)
            unset(_axmol_var_value)
            if(AX_GLES_PROFILE OR AX_USE_GL OR AX_USE_METAL)
                message(STATUS "axmol 引擎：着色器编译参数取自预编译树"
                               "（AX_GLES_PROFILE=${AX_GLES_PROFILE} AX_USE_GL=${AX_USE_GL} "
                               "AX_USE_METAL=${AX_USE_METAL}）")
            else()
                message(STATUS "axmol 引擎：预编译树缓存里没有着色器语言相关变量；若本工程含自定义着色器，"
                               "axslcc 可能报 \"Shader language is not specified\"")
            endif()
        endif()

        # 迁移清理：从源码模式切到预编译模式后，输出目录里可能残留源码模式创建的 axslc
        # 符号链接/联接（源码模式用 SYM_LINK，预编译模式改成复制）。留着链接会让复制
        # 变成“把目录复制到它自己”，fsync 直接报错导致构建中断。
        # 这里只删除链接本身，不影响它指向的目录（file(REMOVE_RECURSE) 对 junction 只删链接）。
        if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.24) # IS_SYMLINK 需要 3.24+
            set(_axmol_axslc_link_dirs "${CMAKE_BINARY_DIR}/bin/${APP_NAME}/axslc")
            foreach(_axmol_cfg IN LISTS CMAKE_CONFIGURATION_TYPES)
                list(APPEND _axmol_axslc_link_dirs "${CMAKE_BINARY_DIR}/bin/${APP_NAME}/${_axmol_cfg}/axslc")
            endforeach()
            foreach(_axmol_axslc_link IN LISTS _axmol_axslc_link_dirs)
                if(EXISTS "${_axmol_axslc_link}" AND IS_SYMLINK "${_axmol_axslc_link}")
                    file(REMOVE_RECURSE "${_axmol_axslc_link}")
                    message(STATUS "axmol 引擎：已清理源码模式残留的 ${_axmol_axslc_link}"
                                   "（预编译模式改为复制，符号链接会导致“复制到自己”失败）")
                endif()
            endforeach()
        endif()

        # 预编译模式生效：以「那棵树自己的缓存」为准决定静态还是动态链接。
        # axmol 的链接助手同样是用 load_cache 读这棵树的选项，这里保持一致，
        # 避免出现“指向动态树却按静态链接”（缺 DLL）这类错配。
        set(_axmol_tree_shared "${AXMOL_ENGINE_SHARED}")
        set(_axmol_tree_cache "${_AX_ROOT}/${AX_PREBUILT_DIR}/CMakeCache.txt")
        if(EXISTS "${_axmol_tree_cache}")
            file(STRINGS "${_axmol_tree_cache}" _axmol_shared_line REGEX "^BUILD_SHARED_LIBS:BOOL=")
            if("${_axmol_shared_line}" MATCHES "=(ON|TRUE|1|YES)$")
                set(_axmol_tree_shared TRUE)
            else()
                set(_axmol_tree_shared FALSE)
            endif()
        endif()

        if(_axmol_tree_shared)
            # 共享方式：axmol 的助手会据此定义 AX_DLLIMPORT 并把引擎 DLL 拷到输出目录
            set(BUILD_SHARED_LIBS TRUE)
        endif()

        if(_axmol_tree_shared AND NOT AXMOL_ENGINE_SHARED)
            message(STATUS "axmol 引擎：使用预编译树 ${AX_PREBUILT_DIR}（动态库方式，按树的实际配置链接；"
                           "建议同时把 AXMOL_ENGINE_SHARED 设为 ON）")
        elseif(NOT _axmol_tree_shared AND AXMOL_ENGINE_SHARED)
            message(STATUS "axmol 引擎：使用预编译树 ${AX_PREBUILT_DIR}（该树是静态库方式，按树的实际配置链接；"
                           "AXMOL_ENGINE_SHARED=ON 只影响下次生成预编译树）")
        elseif(_axmol_tree_shared)
            message(STATUS "axmol 引擎：使用预编译树 ${AX_PREBUILT_DIR}（动态库方式，本工程不再从源码编译引擎）")
        else()
            message(STATUS "axmol 引擎：使用预编译树 ${AX_PREBUILT_DIR}（静态库方式，本工程不再从源码编译引擎）")
        endif()
    else()
        message(STATUS "axmol 引擎：未启用预编译，本次仍从源码编译引擎")
        message(STATUS "axmol 引擎：执行一次 `cmake --build \"${CMAKE_BINARY_DIR}\" --target axmol_engine_prebuilt` "
                       "可生成预编译树 ${_axmol_engine_tree}，之后重新配置 CMake 即生效")
    endif()

    # 用与本工程完全相同的生成器/平台/工具集/配置去配置引擎工程
    set(_axmol_engine_cfg_cmd
        "${CMAKE_COMMAND}" -S "${_AX_ROOT}" -B "${_AX_ROOT}/${_axmol_engine_tree}"
        -G "${CMAKE_GENERATOR}")
    if(CMAKE_GENERATOR_PLATFORM)
        list(APPEND _axmol_engine_cfg_cmd -A "${CMAKE_GENERATOR_PLATFORM}")
    endif()
    if(CMAKE_GENERATOR_TOOLSET)
        list(APPEND _axmol_engine_cfg_cmd -T "${CMAKE_GENERATOR_TOOLSET}")
    endif()
    if(NOT CMAKE_CONFIGURATION_TYPES)
        list(APPEND _axmol_engine_cfg_cmd "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
    endif()
    # 引擎自带的测试/示例对本工程没有意义
    list(APPEND _axmol_engine_cfg_cmd "-DAX_BUILD_TESTS=OFF")
    # 链接方式（静态/动态）显式传给引擎构建，保证与预编译树的实际配置一致
    if(AXMOL_ENGINE_SHARED)
        list(APPEND _axmol_engine_cfg_cmd "-DBUILD_SHARED_LIBS=TRUE")
    else()
        list(APPEND _axmol_engine_cfg_cmd "-DBUILD_SHARED_LIBS=FALSE")
    endif()

    # axmol 里有些符号没有标 AX_DLL，共享构建不会把它们放进导出表，而 app 侧又会引用到：
    #   - core/renderer/backend/Types.h 的 UniformLocation / StageUniformLocation
    #     （operator== / operator() 定义在 Types.cpp，结构体本身没有 AX_DLL）
    #   - core/renderer/Renderer.h:431 的「内嵌类」TriangleCommandBufferManager
    #     （外层 class AX_DLL Renderer 的 dllexport 不覆盖内嵌类）
    # 结果是 app 链接期 LNK2001（约 3 个函数符号）。
    # CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 会让 CMake 从目标自身的 obj 生成 .def 导出全部符号，
    # 覆盖这些漏标处；它只影响 DLL 的链接步骤，不会触发引擎源码重编译。
    if(AXMOL_ENGINE_SHARED AND WIN32)
        list(APPEND _axmol_engine_cfg_cmd "-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=ON")
    endif()

    # axmol 自身的一处不一致，动态库模式必须绕开：
    #   引擎的预编译头就是 axmol.h（core/CMakeLists.txt 的 target_precompile_headers），
    #   而 axmol.h 无条件包含 3d/*.h；若 AX_ENABLE_3D=FALSE，3d/*.cpp 不参与编译，
    #   则 PCH 对象引用的 3D 符号在链接 DLL 时无人定义
    #   （静态库无害：归档成员没被引用就不会被拉入），表现为 axmol.dll 的 LNK2019：
    #   Animation3D / Plane / Frustum / Mesh / MeshSkin / Skeleton3D / Skybox / Animate3D 等。
    # 因此动态库版引擎强制开启 3D（代价仅为多编译 core/3d 下若干源文件）。
    set(_axmol_force_3d FALSE)
    if(AXMOL_ENGINE_SHARED AND NOT AX_ENABLE_3D)
        set(_axmol_force_3d TRUE)
        message(STATUS "axmol 引擎：动态库版需要 3D 符号（axmol.h 无条件包含 3d 头且被用作引擎 PCH），"
                       "本次为引擎构建强制 AX_ENABLE_3D=TRUE；预编译模式还会把这个值带回到本工程")
    endif()

    # 转发本工程的引擎选项，保证预编译树与源码模式的引擎配置一致
    foreach(_axmol_opt IN LISTS AXMOL_ENGINE_OPTIONS)
        if(_axmol_force_3d AND _axmol_opt STREQUAL "AX_ENABLE_3D")
            continue() # 上面已决定强制开启，避免再传 FALSE 覆盖
        endif()
        if(DEFINED ${_axmol_opt} AND NOT "${${_axmol_opt}}" STREQUAL "")
            list(APPEND _axmol_engine_cfg_cmd "-D${_axmol_opt}=${${_axmol_opt}}")
        endif()
    endforeach()
    if(_axmol_force_3d)
        list(APPEND _axmol_engine_cfg_cmd "-DAX_ENABLE_3D=TRUE")
    endif()

    # ── 预编译树体检：选项是否与本次构建一致、产物是否齐全 ──────────────────────
    # 预编译树是「上一次的选项」构建出来的。一旦本工程的选项变了（例如打开某个扩展），
    # app 的链接行会引用树里还不存在的库，报出来却只是一句难懂的：
    #   LINK : fatal error LNK1104: 无法打开文件"cocostudio.lib"
    # 所以在 configure 阶段就把话说清楚，并给出该做什么。
    set(_axmol_tree_cache_file "${_AX_ROOT}/${_axmol_engine_tree}/CMakeCache.txt")
    set(_axmol_tree_problems "")
    if(EXISTS "${_axmol_tree_cache_file}")
        # 1) 选项一致性：以本次要传给引擎的 -D 值为准（含上面强制开启的 3D）
        foreach(_axmol_arg IN LISTS _axmol_engine_cfg_cmd)
            if(_axmol_arg MATCHES "^-D(AX_[A-Za-z0-9_]+)=(.*)$")
                set(_axmol_opt_name "${CMAKE_MATCH_1}")
                axmol_normalize_bool(_axmol_opt_want "${CMAKE_MATCH_2}")
                file(STRINGS "${_axmol_tree_cache_file}" _axmol_have_lines
                     REGEX "^${_axmol_opt_name}:[^=]*=")
                if(_axmol_have_lines)
                    list(GET _axmol_have_lines 0 _axmol_have_line)
                    string(REGEX REPLACE "^[^=]*=" "" _axmol_have_raw "${_axmol_have_line}")
                    axmol_normalize_bool(_axmol_opt_have "${_axmol_have_raw}")
                    if(NOT "${_axmol_opt_have}" STREQUAL "${_axmol_opt_want}")
                        list(APPEND _axmol_tree_problems
                             "${_axmol_opt_name}（树里=${_axmol_opt_have}，本次=${_axmol_opt_want}）")
                    endif()
                endif()
            endif()
        endforeach()
    endif()

    # 2) 产物齐全性：axmol 的助手会链接/拷贝这些引擎库与 DLL，缺一个构建就会失败
    if(WIN32)
        set(_axmol_expected_libs
            axmol glfw glad cocostudio particle3d physics-nodes spine assets-manager GUI)
        set(_axmol_expected_dlls
            glad glfw pugixml freetype axmol cocostudio GUI particle3d physics-nodes spine assets-manager)
        set(_axmol_missing_artifacts "")
        foreach(_axmol_lib IN LISTS _axmol_expected_libs)
            if(NOT EXISTS "${_AX_ROOT}/${_axmol_engine_tree}/lib/${_axmol_lib}.lib")
                list(APPEND _axmol_missing_artifacts "lib/${_axmol_lib}.lib")
            endif()
        endforeach()
        foreach(_axmol_dll IN LISTS _axmol_expected_dlls)
            if(NOT EXISTS "${_AX_ROOT}/${_axmol_engine_tree}/bin/${_axmol_dll}.dll")
                list(APPEND _axmol_missing_artifacts "bin/${_axmol_dll}.dll")
            endif()
        endforeach()
        if(_axmol_missing_artifacts)
            list(APPEND _axmol_tree_problems "缺少产物 ${_axmol_missing_artifacts}")
        endif()
    endif()

    if(_axmol_tree_problems)
        message(WARNING
            "axmol 引擎：预编译树 ${_axmol_engine_tree} 需要重建，原因：${_axmol_tree_problems}。\n"
            "请先构建目标 axmol_engine_prebuilt（CLion 的 Build 窗口把 target 切到它，"
            "或执行 cmake --build <构建目录> --target axmol_engine_prebuilt），再构建本工程；"
            "否则会出现 LNK1104 / POST_BUILD 拷贝失败之类的报错。\n"
            "引擎选项集中在 cmake/modules/AxmolEngineOptions.cmake。")
    endif()

    add_custom_target(axmol_engine_prebuilt
        COMMAND ${_axmol_engine_cfg_cmd}
        COMMAND "${CMAKE_COMMAND}" --build "${_AX_ROOT}/${_axmol_engine_tree}" --config $<CONFIG> --parallel
        COMMENT "构建 axmol 引擎到 ${_axmol_engine_tree}（通常只需一次；引擎选项变更后需重跑）"
        USES_TERMINAL
        VERBATIM)
endif()
