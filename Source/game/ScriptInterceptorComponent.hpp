//
// Created by limen on 2024/11/22.
//

#ifndef SCRIPTINTERCEPTORCOMPONENT_HPP
#define SCRIPTINTERCEPTORCOMPONENT_HPP

#include <axmol.h>

#include <limits>

#include "2dfm/2dfmCommon.hpp"
#include "engine/KgtComponent.hpp"

struct CommonResource;

namespace _2dfm {
    struct PlaySoundCmd;
    struct ShowPic;
    struct ColorSetCmd;
    struct MoveCmd;
    struct ObjectCmd;
    struct AfterimageCmd;
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

    /**
     * 脚本推进（含跳帧补偿）统一在这里做。子类不要再各写一份：
     * 时序逻辑一旦分叉，某一路少了补偿就会整体拖慢，很难发现。
     */
    void update(float delta) override;

    void initRunningScript(int scriptIdx, int offset = 0);

    virtual const CommonResource *getCommonResource() const = 0;

    /// 停止解析执行
    virtual void stop();

    /**
     * 当前脚本是否已经“播完”（本次执行终止）。
     *
     * 三种情况：
     *   - 遇到了“完”脚本项
     *   - 整个脚本里一个“图”项都没有（hasNoShowPicItem）
     *   - Player 专属：脚本走到末尾（末尾有一个隐形的“完”，见 shouldLoopAtScriptEnd）
     *
     * 注意“图的停留时间为 0”**不算**播完：那表示这张图无限等待、脚本执行就阻塞在那里，
     * 属于数据刻意安排的阻塞（以后由“招”脚本按别的条件介入打断）。
     */
    bool isScriptFinished() const { return scriptFinished; }

protected:
    bool hasNoShowPicItem() const;

    /// 每帧在“推进脚本之前”调用（默认什么都不做）。Demo 用它累计播放时间。
    virtual void onAdvanceTick(float deltaTime) {}

    /**
     * 脚本走到末尾（整段都没遇到“完”）时，是否回到开头继续循环。
     *
     * 场景 / KGT / DEMO：true —— 没遇到“完”就是循环播放；遇到“完”才结束。
     * Player：false —— 相当于脚本末尾有一个隐形的“完”：本次执行到此结束。
     *          “完”对 Player 只代表这一段执行完了，接下来播什么由状态机（CharacterController）
     *          在 pickNextScriptAfterFinish() 里决定，解释器本身不退出，节点也不销毁。
     */
    virtual bool shouldLoopAtScriptEnd() const { return true; }

    /**
     * 脚本执行结束时，是否清空纹理并隐藏精灵。
     *
     * 场景 / KGT / DEMO：true —— 脚本结束意味着该脚本对象要销毁，不该再画。
     * Player 主角：false —— 保留当前画面，等状态机切到下一段脚本。
     *   注意 onShowPic 是“沿用当前可见性”的（interceptShowPicCmd 里把 visible 原样设回去），
     *   所以这里一旦隐藏，后续脚本的“图”也照不亮它了，主角绝不能在这里被隐藏。
     * Player 的“物”：true —— 物体脚本结束，物体就该消失。
     */
    virtual bool shouldClearSpriteOnScriptEnd() const { return true; }

    /**
     * “这一段脚本执行完了”时，问上层接下来播什么。
     *
     * 返回 >= 0：立刻切到该脚本并继续解释执行 —— 同一个调用里就出新脚本的图，
     *            也就是说下一段脚本**当帧生效**，不会白停一帧；
     * 返回 < 0 ：本次执行到此结束（收尾见 shouldClearSpriteOnScriptEnd / onScriptStopped）。
     *
     * 默认 -1：场景 / KGT / DEMO 不接这一套，它们靠 shouldLoopAtScriptEnd 循环。
     */
    virtual int pickNextScriptAfterFinish() { return -1; }

    /// 脚本结束、且上层没有再给出下一段脚本时调用（Player 的“物”用它标记回收）
    virtual void onScriptStopped() {}

    /// 按 shouldClearSpriteOnScriptEnd() 的决定收尾（清空纹理并隐藏）
    void clearSpriteOnScriptEnd();
    /// 向 pickNextScriptAfterFinish() 要下一段脚本并接上，接上返回 true
    bool continueWithNextScript();

    /// 解释执行直到遇到图片脚本（触发等待）
    _2dfm::ShowPic *interceptScriptUntilShowPic();

    void jumpToScriptItem(int scriptIdx, int offset = 0);
    void pushRunningScript(int scriptIdx, int offset = 0, int loop = 1);

    void interceptPlaySoundCmd(const _2dfm::PlaySoundCmd *cmd);
    virtual void interceptShowPicCmd(const _2dfm::ShowPic *cmd);
    void interceptColorSetCmd(const _2dfm::ColorSetCmd *cmd);
    void interceptMoveCmd(const _2dfm::MoveCmd *cmd) const;
    virtual void interceptObjectCmd(const _2dfm::ObjectCmd *cmd) const;
    void interceptAfterimageCmd(const _2dfm::AfterimageCmd *cmd);

    ax::Sprite *spriteComponent = nullptr;
    class MoveComponent *moveComponent = nullptr;

    /// 执行栈，用于处理循环和调用指令
    std::vector<ScriptRunningInfo> runningStack;
    int runningScriptItemIdx = 0;
    int originalScriptIdx = 0;

    float timeWaiting = 0;

    /// 见 isScriptFinished()
    bool scriptFinished = false;
};

#endif //SCRIPTINTERCEPTORCOMPONENT_HPP
