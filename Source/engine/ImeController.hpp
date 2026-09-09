#pragma once

namespace ax
{
class RenderView;
}

class ImeController {
public:
    /// 控制游戏窗口输入法(IME) 是否参与组合输入。
    /// @param enabled false 时断开窗口与输入法上下文(HIMC) 的关联，输入法不再拦截按键
    ///                （等效于该窗口输入法保持"英文直通"，纯键盘游戏无需手动 Shift 切英文）；
    ///                true 时用首次禁用前保存的上下文重新关联，恢复中文输入。
    /// @note 仅在 Windows 桌面有实现，其他平台为空操作。只作用于传入的游戏窗口，不改变
    ///       系统输入法设置，也不影响同进程其他窗口（如文件选择对话框仍可输入中文）。
    static void setWindowImeEnabled(ax::RenderView* view, bool enabled);
};
