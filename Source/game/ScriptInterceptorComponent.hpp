//
// Created by limen on 2024/11/22.
//

#ifndef SCRIPTINTERCEPTORCOMPONENT_HPP
#define SCRIPTINTERCEPTORCOMPONENT_HPP

#include <axmol.h>

#include <limits>

#include "2dfm/2dfmCommon.hpp"
#include "2dfm/2dfmScriptItem.hpp"
#include "engine/KgtComponent.hpp"

struct CommonResource;

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

    /**
     * 通知解释器：某个“招”条件已经满足（目前只接“着地后”，由解释器在着地判定里调用，见 update）。
     *
     * 返回是否真的触发了跳转。命中一条监听时：
     *   - 跳到登记时记下的位置（目标脚本 + 该脚本内的偏移），并立刻解释到新脚本的第一张“图”，
     *     当帧生效，不会白停一帧；
     *   - 顺带解除“停留时间为 0 的图”造成的无限等待 —— “招”正是用来打断它的（见 update）；
     *   - 这条监听被消耗掉（“着地后”的监听命中一次就清空，要再触发得由脚本重新登记）。
     *
     * 注意这里**不**调 onScriptJumped()：Player 把“通常脚本发生跳转”当成进入攻击姿势，
     * 而“招”的跳转是招式内部/脚本自己的流程（例如落地接后续动作），不代表转成攻击。
     */
    bool notifyMoveBranch(_2dfm::MoveBranchCmd::Cond cond);

    /**
     * 触地事件：由 CharacterController 在真正落地的那一刻调用（每帧不轮询）。
     *
     * 落地这一下按顺序判两件事：
     *   1. 脚本登记过“招：着地后” → 跳转到它指定的位置，并清掉这条监听；
     *   2. 没有，而实际速度仍然朝下 → 这一段移动撞地到头了，按“完”处理（交给上层状态机）。
     *
     * 只有“落地”这一下才算事件：角色本来就站在地上时，登记“着地后”不会立刻生效，
     * 得先真的跳起来（离开地面）再落地 —— 与 2DFM 的实际行为一致。
     */
    void notifyLanded();

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
     * 属于数据刻意安排的阻塞（由“招”脚本按别的条件介入打断，见 notifyMoveBranch）。
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
     * 已经落地、而实际速度仍然朝下（MoveComponent 的 velocity.y > 0，逻辑坐标 Y 向下为正）。
     *
     * 速度是引擎积分出来的结果（velocity += acceleration × dt），而“移”的作用是继承的，
     * 所以这个速度只可能来自脚本自己下的“移”—— 说明这一段移动还在把角色往地面里压。
     */
    bool shouldFinishAtGround() const;

    /**
     * 结束本次脚本执行，语义等于遇到“完”（“已着陆仍向下移动”走的就是这里）。
     *
     * allowHandoff = false 时不再向上层要下一段脚本：本次调用已经无缝接过一段，
     * 再接力就可能形成死循环（与“完”的处理一致）。
     * 返回 true 表示上层给了下一段脚本并且已经接上，调用方继续解释执行即可。
     */
    bool finishScriptExecution(bool allowHandoff = true);

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

    /**
     * 解释执行中发生了“跳到别的脚本”时调用（跳 / 调 / 循 / 随 命中）。
     *
     * 对 Player 来说：通常状态的脚本（站立、行走、下蹲……）一旦用了这类跳转，
     * 就说明它把执行交给了别的脚本，此时要当作进入了「攻击」状态 ——
     * 在脚本执行完之前不再响应输入，播完再由结束回调交还给通常状态。
     */
    virtual void onScriptJumped() {}

    /// 按 shouldClearSpriteOnScriptEnd() 的决定收尾（清空纹理并隐藏）
    void clearSpriteOnScriptEnd();
    /// 向 pickNextScriptAfterFinish() 要下一段脚本并接上，接上返回 true
    bool continueWithNextScript();

    /// 解释执行直到遇到图片脚本（触发等待）
    _2dfm::ShowPic *interceptScriptUntilShowPic();

    /**
     * 从当前位置一路解释执行到下一张“图”并把它应用上去（含跳帧补偿），
     * 过程中遇到的“声 / 色 / 移 / 跳 / 调 / 循 / 随 / 物 / 残”等照常执行。
     *
     * 由 update() 每帧调用；“招”命中后也要走它，跳过去的那段才会当帧出图。
     * 本次执行终止时返回 false（调用方据此置为无限等待）。
     */
    bool advanceUntilShowPic();

    void jumpToScriptItem(int scriptIdx, int offset = 0);
    void pushRunningScript(int scriptIdx, int offset = 0, int loop = 1);

    void interceptPlaySoundCmd(const _2dfm::PlaySoundCmd *cmd);
    virtual void interceptShowPicCmd(const _2dfm::ShowPic *cmd);
    void interceptColorSetCmd(const _2dfm::ColorSetCmd *cmd);
    void interceptMoveCmd(const _2dfm::MoveCmd *cmd) const;
    virtual void interceptObjectCmd(const _2dfm::ObjectCmd *cmd) const;
    void interceptAfterimageCmd(const _2dfm::AfterimageCmd *cmd);

    /**
     * 登记一条“招”监听（见 MoveBranchCmd）：同一个条件只保留一条，重复遇到按“重新设置”覆盖，
     * 免得脚本回头循环时把监听堆起来。“无”不登记。
     */
    void interceptMoveBranchCmd(const _2dfm::MoveBranchCmd *cmd);
    /// 清空“招”监听：开始新一轮执行、脚本执行终止、或停止解析时调用
    void clearMoveBranchListeners();

    ax::Sprite *spriteComponent = nullptr;
    class MoveComponent *moveComponent = nullptr;

    /// 执行栈，用于处理循环和调用指令
    std::vector<ScriptRunningInfo> runningStack;
    int runningScriptItemIdx = 0;
    int originalScriptIdx = 0;

    /// 一条“招”监听：条件 + 命中后要跳到的位置
    struct MoveBranchListener {
        _2dfm::MoveBranchCmd::Cond condition = _2dfm::MoveBranchCmd::Cond::NONE;
        int targetScriptId = 0;
        int targetPos = 0;
    };

    /// “招”监听器，随当前这一轮脚本执行存活（见 notifyMoveBranch）
    std::vector<MoveBranchListener> moveBranchListeners;

    float timeWaiting = 0;

    /// 见 isScriptFinished()
    bool scriptFinished = false;
};

#endif //SCRIPTINTERCEPTORCOMPONENT_HPP
