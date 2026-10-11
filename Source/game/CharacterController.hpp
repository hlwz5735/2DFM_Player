//
// CharacterController: 角色控制器（状态机）
//
// 角色的状态分成互不干扰的两套：
//   1) 位置状态：下蹲 / 站立 / 浮空  —— 角色此刻“处在什么空间形态”
//   2) 姿势状态：通常 / 挨打 / 防御 / 攻击 —— 角色此刻“在做什么类别的事”
// 两者是正交的（例如“浮空 + 挨打”“下蹲 + 防御”都成立），所以分开成两个枚举。
//
// 本版只实现「通常」姿势：位置状态由方向键驱动，脚本编排全部写在这里。
// 挨打 / 防御 / 攻击属于后续扩展点，见 switchPose()。
//
// ── 与脚本解释器的关系（重要） ──
// 角色的脚本执行器（PlayerScriptInterceptor）在“这一段脚本执行完”时会回调
// pickNextScriptAfterFinish()：由状态机决定下一段脚本并立刻返回脚本 ID，
// 解释器在同一个调用里无缝切过去并继续出图 —— 立即生效，不保留到下一帧。
// 所以这里**不要**用“每帧轮询脚本是否播完”的方式推进过渡：那样会白白多停一帧。
// 每帧的 update 只负责三件事：读输入、看角色有没有真的升空、看有没有触地。
//

#ifndef CHARACTERCONTROLLER_HPP
#define CHARACTERCONTROLLER_HPP

#include <axmol.h>

#include <limits>
#include <memory>

#include "2dfm/KgtPlayer.hpp"
#include "engine/Input.hpp"
#include "engine/KgtComponent.hpp"

/// 位置状态
enum class CharacterPositionState {
    Stand,    // 站立
    Crouch,   // 下蹲
    Airborne, // 浮空
};

/// 姿势状态
enum class CharacterPoseState {
    Normal, // 通常
    Hurt,   // 挨打
    Guard,  // 防御
    Attack, // 攻击
};

/**
 * 「通常」姿势下的具体动作。
 *
 * 位置状态只区分“站在地上 / 蹲着 / 在空中”，但同一个位置状态下还要播不同的脚本
 * （例如站在地上可能是待机、走、下蹲过渡、起跳过渡），所以再分一层动作。
 * 位置状态由动作推导，动作推进时位置状态随之变化。
 */
enum class CharacterNormalAction {
    Idle,         // 站立待机：反复播放“站立”
    WalkForward,  // 前行（角色面向右时按右）
    WalkBackward, // 后退（角色面向右时按左）
    CrouchDown,   // 下蹲过渡：播放“下蹲”，播完转 CrouchHold
    CrouchHold,   // 蹲着：持续播放“蹲着”
    StandUp,      // 从下蹲站起：播放“从下蹲站起”，播完回 Idle
    JumpUp,       // 上跳：播放“上跳”，Y 轴一开始上升就转浮空
    JumpForward,  // 前跳
    JumpBackward, // 后跳
    Falling,      // 空中落下：跳跃脚本播完后播放
};

const char *toString(CharacterPositionState state);
const char *toString(CharacterPoseState state);
const char *toString(CharacterNormalAction action);

class PlayerNode;
class PlayerScriptInterceptor;

class CharacterController : public KgtComponent {
public:
    bool init() override;
    void update(float delta) override;

    void setPlayerData(std::shared_ptr<KgtPlayer> data);

    /**
     * 地面线的逻辑 Y 坐标（逻辑坐标 Y 轴向下为正）。
     * 不设置时以角色挂上控制器时的逻辑坐标作为地面线。
     */
    void setGroundY(float y) { groundY = y; }
    float getGroundY() const { return groundY; }

    CharacterPositionState getPositionState() const { return positionState; }
    CharacterPoseState getPoseState() const { return poseState; }
    CharacterNormalAction getNormalAction() const { return normalAction; }
    int getCurrentScriptId() const { return currentScriptId; }

    class PlayerNode *getPlayerNode() const;
    PlayerScriptInterceptor *getInterceptor() const;

    /// 立刻播放指定脚本（不做状态判定）。供外部编排或后续“招”脚本使用。
    void playScript(int scriptId);

    /**
     * 由脚本解释器在“当前这一段脚本执行完了”时调用（同一帧内立即生效）。
     *
     * 内部按当前的姿势状态 / 位置状态 / 动作推进，返回接下来要播的脚本 ID；
     * 返回 -1 表示不再有下一段。
     * 这里只做“决策 + 改自己的状态”，真正起脚本由调用方完成，
     * 避免在解释执行过程中重入地重置执行栈。
     */
    int pickNextScriptAfterFinish();

    /**
     * 切换姿势状态。
     * 目前只实现 Normal；挨打 / 防御 / 攻击需要各自的脚本编排与“招”数据支撑，
     * 尚未实现，调用会被忽略并打一条警告。
     */
    void switchPose(CharacterPoseState pose);

private:
    /**
     * 每帧更新只做三件事：读输入、看角色有没有真的升空、看有没有触地。
     *
     * 注意：脚本的推进完全由解释器的“脚本结束”回调负责，这里**不做**“脚本是否播完”的轮询。
     * 另外，“图的停留时间为 0”表示这张图无限等待、脚本执行就阻塞在那里（数据本意如此，
     * 以后由“招”脚本按别的条件介入打断），所以也**不能**把它当作“播完”去推进。
     */
    void updateNormalPose(float delta);
    void updateNormalStand(float delta);
    void updateNormalCrouch(float delta);
    void updateNormalAirborne(float delta);

    /// “脚本结束后播什么”的通常姿势版（由 pickNextScriptAfterFinish 分派）
    int pickNextNormalScriptAfterFinish();

    /// 只改状态与当前脚本 ID，不起脚本（返回脚本 ID，非法则返回 -1）
    int decideNormalAction(CharacterNormalAction action);
    /// 改状态并立刻起脚本（输入驱动用）
    void startNormalAction(CharacterNormalAction action);

    /// 角色是否已经有向上的动向（逻辑坐标真的升高，或脚本给了向上的速度）
    bool isLiftingOff() const;
    int scriptIdForNormalAction(CharacterNormalAction action) const;
    static bool isJumpAction(CharacterNormalAction action);

    /// 从站立起跳后真正升空（位置状态改为浮空，跳跃脚本继续播）
    void enterAirborne(CharacterNormalAction action);
    /// 落地的“只改状态”部分：贴地、清掉垂直速度与加速度、位置状态回站立
    void resetAirborneOnLanding();
    /// 落地并起“站立”脚本（由每帧的触地判定调用）
    void land();

    std::shared_ptr<KgtPlayer> playerData;
    mutable class PlayerNode *playerNode = nullptr;
    mutable PlayerScriptInterceptor *interceptor = nullptr;

    CharacterPositionState positionState = CharacterPositionState::Stand;
    CharacterPoseState poseState = CharacterPoseState::Normal;
    CharacterNormalAction normalAction = CharacterNormalAction::Idle;
    /// 当前这一段脚本的 ID（脚本结束回调与兜底推进都要靠它判断“是不是同一段”）
    int currentScriptId = -1;

    float groundY = std::numeric_limits<float>::quiet_NaN();
    /// 浮空期间是否真的离开过地面（位置真的升高过），用于“触地”判定
    bool leftGround = false;
    /// 第一帧才真正开始驱动状态机（那时节点已经挂到场景上、坐标也就位了）
    bool started = false;
};

#endif //CHARACTERCONTROLLER_HPP
