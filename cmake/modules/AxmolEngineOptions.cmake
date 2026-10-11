# ============================================================================
# axmol 引擎编译选项（本工程唯一的选项来源）
#
# 这些选项在两种模式下必须一致：
#   1) 源码模式    ：引擎作为子项目编进本工程（add_subdirectory(${_AX_ROOT}/core)）
#   2) 预编译模式 ：它们会被原样转发给独立的引擎构建，见 AxmolPrebuilt.cmake
#
# 因此新增或修改引擎选项时只改这里一处，不要分散到别的地方。
# ============================================================================

set(AX_ENABLE_MSEDGE_WEBVIEW2 FALSE)
set(AX_ENABLE_MFMEDIA FALSE)
set(AX_ENABLE_VLC_MEDIA FALSE)
set(AX_ENABLE_3D FALSE)
set(AX_ENABLE_3D_PHYSICS FALSE)
set(AX_ENABLE_NAVMESH FALSE)
set(AX_ENABLE_MEDIA FALSE)
set(AX_USE_ALSOFT TRUE)
# 以下四个扩展必须开启（共享模式）：
#   axmol 的预编译链接助手（AXLinkHelpers.cmake:244-259）用一条 copy_if_different
#   无条件拷贝 glad/glfw/pugixml/freetype/axmol/cocostudio/GUI/particle3d/physics-nodes/
#   spine/assets-manager 这 11 个 DLL，而 copy_if_different 对「源文件不存在」会硬报错。
#   关掉它们就会让共享模式的 POST_BUILD 直接失败，因此按 axmol 的假定打开
#   （代价：引擎多编译约 317 个源文件，一次性；多出 4 个本工程不使用的 DLL）。
set(AX_ENABLE_EXT_PARTICLE3D TRUE)
set(AX_ENABLE_EXT_PHYSICS_NODE TRUE)
set(AX_ENABLE_EXT_SPINE TRUE)
set(AX_ENABLE_EXT_COCOSTUDIO TRUE)
set(AX_ENABLE_EXT_DRAGONBONES FALSE)
set(AX_ENABLE_EXT_FAIRYGUI FALSE)
set(AX_ENABLE_EXT_SDFGEN FALSE)
set(AX_ENABLE_EXT_LUA FALSE)

# 需要转发给独立引擎构建的选项清单（与上面一一对应）
set(AXMOL_ENGINE_OPTIONS
    AX_ENABLE_MSEDGE_WEBVIEW2
    AX_ENABLE_MFMEDIA
    AX_ENABLE_VLC_MEDIA
    AX_ENABLE_3D
    AX_ENABLE_3D_PHYSICS
    AX_ENABLE_NAVMESH
    AX_ENABLE_MEDIA
    AX_USE_ALSOFT
    AX_ENABLE_EXT_PARTICLE3D
    AX_ENABLE_EXT_PHYSICS_NODE
    AX_ENABLE_EXT_SPINE
    AX_ENABLE_EXT_DRAGONBONES
    AX_ENABLE_EXT_COCOSTUDIO
    AX_ENABLE_EXT_FAIRYGUI
    AX_ENABLE_EXT_SDFGEN
    AX_ENABLE_EXT_LUA
)
