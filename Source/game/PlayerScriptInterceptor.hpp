//
// Created by limen on 2025/1/9.
//

#ifndef PLAYERSCRIPTINTERCEPTOR_HPP
#define PLAYERSCRIPTINTERCEPTOR_HPP

#include <axmol.h>
#include <memory>

#include "2dfm/KgtPlayer.hpp"
#include "ScriptInterceptorComponent.hpp"

class PlayerScriptInterceptor : public ScriptInterceptorComponent {
public:
    const CommonResource *getCommonResource() const override { return playerData.get(); }

    void setPlayerData(std::shared_ptr<KgtPlayer> playerData);
    class PlayerNode *getPlayerNode() const;
protected:
    void interceptObjectCmd(const _2dfm::ObjectCmd *cmd) const override;

    /**
     * Player 的脚本执行器在脚本最末尾有一个隐形的“完”：
     * 走到末尾就算本次执行结束，不回头循环（场景 / KGT / DEMO 才循环）。
     * “完”对 Player 只代表这一段执行完了，接下来播什么脚本由状态机决定，解释器绝不退出。
     */
    bool shouldLoopAtScriptEnd() const override { return false; }

    /**
     * 主角脚本结束时保留当前画面（等状态机切下一段脚本），不能在结束处清空精灵：
     * “图”脚本项是沿用当前可见性的，一旦在这里隐藏，后面新脚本的“图”也照不亮它。
     * 没有状态机的（“物”、选人界面展示）则相反：脚本结束就该消失，随后整节点回收。
     */
    bool shouldClearSpriteOnScriptEnd() const override;

    /**
     * 脚本执行完了，问同一节点上的状态机（CharacterController）接下来播什么：
     * 它给出的脚本会被立刻接上，当帧生效（换脚本、或者跳回开头继续播）。
     * 没有状态机（“物”、选人界面展示）就返回 -1，本次执行到此结束。
     */
    int pickNextScriptAfterFinish() override;

    /// 脚本结束且没有状态机接管：这个脚本执行器的终点就是销毁，先标记回收
    void onScriptStopped() override;

    /// 脚本里发生了“跳 / 调 / 循”等跳转：交给状态机处理（通常状态 → 攻击状态）
    void onScriptJumped() override;

    /// 同一个节点上的状态机；没有则为 nullptr（“物”、选人界面展示都属于没有）
    class CharacterController *findCharacterController() const;
private:
    std::shared_ptr<KgtPlayer> playerData;
    mutable class PlayerNode *playerNode = nullptr;
    /// 缓存“有没有状态机”的查询结果（组件在节点生命周期内不会变）
    mutable class CharacterController *characterController = nullptr;
    mutable bool controllerResolved = false;
};


#endif //PLAYERSCRIPTINTERCEPTOR_HPP
