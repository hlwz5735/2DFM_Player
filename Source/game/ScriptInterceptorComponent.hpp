//
// Created by limen on 2024/11/22.
//

#ifndef SCRIPTINTERCEPTORCOMPONENT_HPP
#define SCRIPTINTERCEPTORCOMPONENT_HPP

#include <axmol.h>

#include "2dfm/2dfmCommon.hpp"
#include "engine/KgtComponent.hpp"

struct CommonResource;
namespace _2dfm {
    struct PlaySoundCmd;
    struct ShowPic;
    struct ColorSetCmd;
    struct MoveCmd;
}

struct ScriptRunningInfo {
    int scriptItemStartIdx = 0;
    int scriptItemEndIdx = 0;
    int stashedScriptItemIdx = 0;

    int originalScriptIdx = 0;
    int originalOffset = 0;
    int loopCount = 1;
};

class ScriptInterceptorComponent : public KgtComponent {
public:
    bool init() override;
    void onAdd() override;

    void initRunningScript(int scriptIdx);

    virtual const CommonResource *getCommonResource() const = 0;

    /// 停止解析执行
    virtual void stop();

protected:
    bool hasNoShowPicItem() const;

    /// 解释执行直到遇到图片脚本（触发等待）
    _2dfm::ShowPic *interceptScriptUntilShowPic();

    void jumpToScriptItem(int scriptIdx, int offset = 0);
    void pushRunningScript(int scriptIdx, int offset = 0, int loop = 1);

    void interceptPlaySoundCmd(const _2dfm::PlaySoundCmd *cmd);
    virtual void interceptShowPicCmd(const _2dfm::ShowPic *cmd);
    void interceptColorSetCmd(const _2dfm::ColorSetCmd *cmd);
    void interceptMoveCmd(const _2dfm::MoveCmd *item) const;

    ax::Sprite *spriteComponent = nullptr;
    class MoveComponent *moveComponent = nullptr;

    /// 执行栈，用于处理循环和调用指令
    std::vector<ScriptRunningInfo> runningStack;
    int runningScriptItemIdx = 0;
    int originalScriptIdx = 0;

    float timeWaiting = 0;
};

#endif //SCRIPTINTERCEPTORCOMPONENT_HPP
