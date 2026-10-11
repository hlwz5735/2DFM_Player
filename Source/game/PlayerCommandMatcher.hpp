//
// PlayerCommandMatcher: 出招指令匹配
//

#ifndef PLAYERCOMMANDMATCHER_HPP
#define PLAYERCOMMANDMATCHER_HPP

#include <memory>

#include "2dfm/KgtPlayer.hpp"

/**
 * 出招指令匹配。
 *
 * 优先级：`KgtPlayer::commands` 里越靠前优先级越高、越靠后越低（与 KGT 数据一致）；
 * 命中的第一条就是本次匹配结果，不再往后看。
 *
 * 本版只处理最简单的形态，复杂的以后再加：
 *   - 只认「只有 1 条指令项」的指令；
 *   - 只认一般步骤（NORMAL），连按 / 蓄力 / 回转先跳过；
 *   - 指令项必须要求按键，且这些按键在这一帧都按着、其中至少一个刚按下
 *     （这样 A+C 这类组合键才按得出来，而不是要求同帧同时按下）；
 *   - 纯方向指令（不带按键，靠方向变化触发）暂不处理；
 *   - 指令受理时间窗 timeWindow 暂不参与判断。
 *
 * 这里只回答「有没有命中哪条指令」，至于命中后用哪个分支脚本（空中 / 蹲着 / 站立远距…），
 * 由状态机按当前位置状态决定 —— 见 CharacterController::scriptIdForCommandState()。
 */
class PlayerCommandMatcher {
public:
    void setPlayerData(std::shared_ptr<KgtPlayer> data) { playerData = std::move(data); }

    struct Match {
        /// 命中的指令；nullptr 表示没有命中
        const PlayerCommand *command = nullptr;

        bool hit() const { return command != nullptr; }
    };

    /// 用当前输入（1P）匹配一条指令
    Match match() const;

private:
    std::shared_ptr<KgtPlayer> playerData;
};

#endif //PLAYERCOMMANDMATCHER_HPP
