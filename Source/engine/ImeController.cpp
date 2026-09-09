#include "engine/ImeController.hpp"
#include <axmol.h>

#if AX_TARGET_PLATFORM == AX_PLATFORM_WIN32
// 背景：本播放器是纯键盘操作的 KGT 演示播放器，运行过程中不需要中文文本输入。若不处理，
// Windows 会让系统输入法（如微软拼音）以用户上次的"中文"模式激活本窗口，此时字母键与
// 方向键会被输入法拦截用于拼字并弹出候选窗，游戏收不到任何键盘事件（需手动 Shift 切英文）。
// 解决机制与 SDL 一致：把窗口关联的输入法上下文(HIMC) 置空后，系统不再对该窗口启用组合输入。

#include <Windows.h>
#include <imm.h>
#pragma comment(lib, "imm32.lib")

#include "platform/RenderViewImpl.h"

void ImeController::setWindowImeEnabled(ax::RenderView* view, bool enabled)
{
    auto impl = dynamic_cast<ax::RenderViewImpl*>(view);
    if (impl == nullptr)
        return;
    HWND hwnd = impl->getWin32Window();
    if (hwnd == nullptr)
        return;

    // 首次禁用时记录窗口原关联的输入法上下文，后续可重新关联以恢复中文输入
    static HIMC originalImeContext = nullptr;
    if (enabled)
        ImmAssociateContext(hwnd, originalImeContext);
    else
        originalImeContext = ImmAssociateContext(hwnd, nullptr);
}

#else

void ImeController::setWindowImeEnabled(ax::RenderView* /*view*/, bool /*enabled*/)
{
    // 非 Windows 桌面平台无输入法拦截按键的问题，无需处理
}

#endif
