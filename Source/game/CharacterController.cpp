//
// CharacterController: 角色控制器（状态机）
//
// 详细的状态说明见 CharacterController.hpp。
//

#include "CharacterController.hpp"

#include <cmath>

#include "MoveComponent.hpp"
#include "PlayerNode.hpp"
#include "PlayerScriptInterceptor.hpp"

USING_NS_AX;

namespace {
/// 判定“已经离地/正在升空”的容差。逻辑坐标 Y 轴向下为正，向上移动即 Y 变小。
constexpr float kLiftoffEpsilon = 0.5f;

/// 切换动作后需要“脚本播完才推进”的过渡动作（这类动作期间不响应方向键）
bool isTransitionAction(CharacterNormalAction action) {
    return action == CharacterNormalAction::CrouchDown || action == CharacterNormalAction::StandUp;
}
} // namespace

const char *toString(CharacterPositionState state) {
    switch (state) {
    case CharacterPositionState::Stand:
        return "站立";
    case CharacterPositionState::Crouch:
        return "下蹲";
    case CharacterPositionState::Airborne:
        return "浮空";
    }
    return "未知";
}

const char *toString(CharacterPoseState state) {
    switch (state) {
    case CharacterPoseState::Normal:
        return "通常";
    case CharacterPoseState::Hurt:
        return "挨打";
    case CharacterPoseState::Guard:
        return "防御";
    case CharacterPoseState::Attack:
        return "攻击";
    }
    return "未知";
}

const char *toString(CharacterNormalAction action) {
    switch (action) {
    case CharacterNormalAction::Idle:
        return "站立";
    case CharacterNormalAction::WalkForward:
        return "前行";
    case CharacterNormalAction::WalkBackward:
        return "后退";
    case CharacterNormalAction::CrouchDown:
        return "下蹲";
    case CharacterNormalAction::CrouchHold:
        return "蹲着";
    case CharacterNormalAction::StandUp:
        return "从下蹲站起";
    case CharacterNormalAction::JumpUp:
        return "上跳";
    case CharacterNormalAction::JumpForward:
        return "前跳";
    case CharacterNormalAction::JumpBackward:
        return "后跳";
    case CharacterNormalAction::Falling:
        return "空中落下";
    }
    return "未知";
}

bool CharacterController::init() {
    if (!KgtComponent::init()) {
        return false;
    }
    // 组件在容器里以名字为键，必须唯一（解释器也靠这个名字找回状态机）
    this->setName("CharacterController");
    return true;
}

void CharacterController::setPlayerData(std::shared_ptr<KgtPlayer> data) {
    this->playerData = std::move(data);
}

PlayerNode *CharacterController::getPlayerNode() const {
    if (playerNode == nullptr) {
        playerNode = dynamic_cast<PlayerNode *>(getOwner());
    }
    return playerNode;
}

PlayerScriptInterceptor *CharacterController::getInterceptor() const {
    if (interceptor == nullptr) {
        if (const auto *node = getPlayerNode()) {
            interceptor = dynamic_cast<PlayerScriptInterceptor *>(node->getInterceptor());
        }
    }
    return interceptor;
}

void CharacterController::playScript(int scriptId) {
    currentScriptId = scriptId;
    if (auto *it = getInterceptor()) {
        it->initRunningScript(scriptId);
    }
}

void CharacterController::switchPose(CharacterPoseState pose) {
    if (pose == poseState) {
        return;
    }
    if (pose != CharacterPoseState::Normal) {
        // 挨打 / 防御 / 攻击各自需要“招”数据与独立的脚本编排，本版先不做
        AXLOGW("CharacterController: 姿势状态 {} 尚未实现，仍保持“{}”",
               toString(pose), toString(poseState));
        return;
    }
    poseState = pose;
}

void CharacterController::update(float delta) {
    KgtComponent::update(delta);

    auto *node = getPlayerNode();
    if (node == nullptr || playerData == nullptr || getInterceptor() == nullptr) {
        return;
    }

    if (!started) {
        started = true;
        // 没有显式指定地面线时，就以角色出场时所在的高度作为地面线
        if (std::isnan(groundY)) {
            groundY = node->getLogicPosition().y;
        }
        // 默认状态：站立 + 通常，反复播放“站立”脚本
        startNormalAction(CharacterNormalAction::Idle);
    }

    switch (poseState) {
    case CharacterPoseState::Normal:
        updateNormalPose(delta);
        break;
    default:
        // 挨打 / 防御 / 攻击：见 switchPose()
        break;
    }
}

// ─────────────────────────── 脚本结束回调（由解释器驱动） ───────────────────────────

int CharacterController::pickNextScriptAfterFinish() {
    switch (poseState) {
    case CharacterPoseState::Normal:
        return pickNextNormalScriptAfterFinish();
    default:
        // 挨打 / 防御 / 攻击：见 switchPose()
        return -1;
    }
}

int CharacterController::pickNextNormalScriptAfterFinish() {
    switch (normalAction) {
    case CharacterNormalAction::CrouchDown:
        // “下蹲”演完 → 转入蹲着
        positionState = CharacterPositionState::Crouch;
        return decideNormalAction(CharacterNormalAction::CrouchHold);

    case CharacterNormalAction::StandUp:
        // “从下蹲站起”演完 → 站立待机
        return decideNormalAction(CharacterNormalAction::Idle);

    case CharacterNormalAction::Idle:
    case CharacterNormalAction::WalkForward:
    case CharacterNormalAction::WalkBackward:
    case CharacterNormalAction::CrouchHold:
    case CharacterNormalAction::Falling:
        // 循环类动作：同一帧无缝接回开头，这就是“反复播放”
        return decideNormalAction(normalAction);

    case CharacterNormalAction::JumpUp:
    case CharacterNormalAction::JumpForward:
    case CharacterNormalAction::JumpBackward:
        // 跳跃动作播完：角色确实起来了就转“空中落下”，没起来就当落地
        if (isLiftingOff()) {
            enterAirborne(normalAction);
            return decideNormalAction(CharacterNormalAction::Falling);
        }
        resetAirborneOnLanding();
        return decideNormalAction(CharacterNormalAction::Idle);
    }
    return -1;
}

// ─────────────────────────── 每帧：输入 / 升空 / 触地 ───────────────────────────

void CharacterController::updateNormalPose(float delta) {
    // 过渡动作（下蹲 / 从下蹲站起）期间不响应方向键，推进完全由解释器的脚本结束回调负责。
    // 若这段脚本的“图”停留时间为 0，脚本就阻塞在那里、状态机也不该自己往前走（数据本意如此）。
    if (isTransitionAction(normalAction)) {
        return;
    }

    switch (positionState) {
    case CharacterPositionState::Stand:
        updateNormalStand(delta);
        break;
    case CharacterPositionState::Crouch:
        updateNormalCrouch(delta);
        break;
    case CharacterPositionState::Airborne:
        updateNormalAirborne(delta);
        break;
    }
}

void CharacterController::updateNormalStand(float delta) {
    // 上跳的起跳阶段：等角色真的被脚本带起来就转浮空（脚本播完的收尾在结束回调里）
    if (normalAction == CharacterNormalAction::JumpUp) {
        if (isLiftingOff()) {
            enterAirborne(CharacterNormalAction::JumpUp);
        }
        return;
    }

    const auto dpad = Input::getInstance().getDPad();

    // ── 下蹲（下、左下、右下）──
    if (Input::isDownDirection(dpad)) {
        startNormalAction(CharacterNormalAction::CrouchDown);
        return;
    }

    // ── 跳（上、左上、右上）──
    // 上类方向“按下或按住”都可以起跳：按住不放时，每次回到站立（落地、站起）都会再跳一次，
    // 这就是连跳 —— 方向取当前方向值，所以左右也按着就跳对应方向。
    // 起跳过程中 / 浮空中不会被重复触发（那两个状态下根本不走这里）。
    if (Input::isUpDirection(dpad)) {
        switch (dpad) {
        case Input::DPad::RightUp:
            startNormalAction(CharacterNormalAction::JumpForward);
            break;
        case Input::DPad::LeftUp:
            startNormalAction(CharacterNormalAction::JumpBackward);
            break;
        default:
            startNormalAction(CharacterNormalAction::JumpUp);
            break;
        }
        // 前跳 / 后跳：脚本一开播就进入浮空通常状态；
        // 上跳则要等角色真的被脚本带起来（见上面的 JumpUp 分支）
        if (normalAction == CharacterNormalAction::JumpForward
            || normalAction == CharacterNormalAction::JumpBackward) {
            enterAirborne(normalAction);
        }
        return;
    }

    // ── 行走：角色面向右时，右=前行，左=后退 ──
    // TODO: 角色转向（站立转身 / 蹲着转身）尚未实现，这里先固定按“面向右”解释
    if (dpad == Input::DPad::Right) {
        if (normalAction != CharacterNormalAction::WalkForward) {
            startNormalAction(CharacterNormalAction::WalkForward);
        }
        return;
    }
    if (dpad == Input::DPad::Left) {
        if (normalAction != CharacterNormalAction::WalkBackward) {
            startNormalAction(CharacterNormalAction::WalkBackward);
        }
        return;
    }

    // ── 方向键回中：站立待机 ──
    if (normalAction != CharacterNormalAction::Idle) {
        startNormalAction(CharacterNormalAction::Idle);
    }
}

void CharacterController::updateNormalCrouch(float delta) {
    const auto dpad = Input::getInstance().getDPad();

    if (!Input::isDownDirection(dpad)) {
        // 松开下方向：回到站立通常，播放“从下蹲站起”
        positionState = CharacterPositionState::Stand;
        startNormalAction(CharacterNormalAction::StandUp);
        return;
    }

    // 持续按住下方向（不论是否有左、右）：持续播放“蹲着”。
    // “蹲着”是循环脚本，播完由解释器的结束回调无缝接回开头，这里不用管。
    // TODO: 蹲着前行 / 蹲着后退（crouchMoveForward/BackwardScriptId）本版按需求先不接
}

void CharacterController::updateNormalAirborne(float delta) {
    // 浮空状态下不响应任何方向键
    auto *node = getPlayerNode();
    const float y = node->getLogicPosition().y;

    // leftGround 只看“位置真的升高过”，免得刚起跳（人还在原地、只有向上速度）就判成落地
    if (y < getGroundY() - kLiftoffEpsilon) {
        leftGround = true;
    }

    // 触地：无条件回到站立通常（后续会改由“招”脚本来决定落地行为）
    if (leftGround && y >= getGroundY()) {
        land();
    }
}

// ─────────────────────────── 动作切换与状态工具 ───────────────────────────

int CharacterController::decideNormalAction(CharacterNormalAction action) {
    normalAction = action;
    if (playerData == nullptr) {
        currentScriptId = -1;
        return -1;
    }
    const int scriptId = scriptIdForNormalAction(action);
    if (scriptId < 0 || scriptId >= static_cast<int>(playerData->scripts.size())) {
        AXLOGW("CharacterController: 动作“{}”没有对应的脚本（scriptId={}）", toString(action), scriptId);
        currentScriptId = -1;
        return -1;
    }
    currentScriptId = scriptId;
    return scriptId;
}

void CharacterController::startNormalAction(CharacterNormalAction action) {
    const int scriptId = decideNormalAction(action);
    if (scriptId >= 0) {
        playScript(scriptId);
    }
}

int CharacterController::scriptIdForNormalAction(CharacterNormalAction action) const {
    switch (action) {
    case CharacterNormalAction::Idle:
        return playerData->standScriptId;
    case CharacterNormalAction::WalkForward:
        return playerData->forwardScriptId;
    case CharacterNormalAction::WalkBackward:
        return playerData->backwardScriptId;
    case CharacterNormalAction::CrouchDown:
        return playerData->crouchScriptId;
    case CharacterNormalAction::CrouchHold:
        return playerData->crouchingScriptId;
    case CharacterNormalAction::StandUp:
        return playerData->standUpScriptId;
    case CharacterNormalAction::JumpUp:
        return playerData->jumpUpScriptId;
    case CharacterNormalAction::JumpForward:
        return playerData->jumpForwardScriptId;
    case CharacterNormalAction::JumpBackward:
        return playerData->jumpBackwardScriptId;
    case CharacterNormalAction::Falling:
        return playerData->fallingScriptId;
    }
    return -1;
}

bool CharacterController::isJumpAction(CharacterNormalAction action) {
    return action == CharacterNormalAction::JumpUp || action == CharacterNormalAction::JumpForward
           || action == CharacterNormalAction::JumpBackward;
}

bool CharacterController::isLiftingOff() const {
    const auto *node = getPlayerNode();
    if (node == nullptr) {
        return false;
    }
    // 逻辑坐标 Y 轴向下为正：位置变小 = 人往上升；速度为负 = 脚本给了向上的速度。
    // 两个都看：位置判定要等这一帧的位移积完，而速度判定在脚本刚下“移”指令时就成立。
    if (node->getLogicPosition().y < getGroundY() - kLiftoffEpsilon) {
        return true;
    }
    const auto *move = node->getMoveComp();
    return move != nullptr && move->getVelocity().y < -kLiftoffEpsilon;
}

void CharacterController::enterAirborne(CharacterNormalAction action) {
    positionState = CharacterPositionState::Airborne;
    // 跳跃脚本继续播，不在这里换脚本
    normalAction = action;
    leftGround = false;
}

void CharacterController::resetAirborneOnLanding() {
    auto *node = getPlayerNode();
    if (node == nullptr) {
        return;
    }
    // 贴地，并清掉垂直方向的速度与加速度，避免落地后继续下沉或被再次带起来
    const auto &pos = node->getLogicPosition();
    node->setLogicPosition(Vec2(pos.x, getGroundY()));
    if (auto *move = node->getMoveComp()) {
        const auto &vel = move->getVelocity();
        const auto &accel = move->getAcceleration();
        move->setVelocity(Vec2(vel.x, 0.f));
        move->setAcceleration(Vec2(accel.x, 0.f));
    }
    leftGround = false;
    positionState = CharacterPositionState::Stand;
}

void CharacterController::land() {
    resetAirborneOnLanding();
    // 每帧的触地判定不是脚本驱动的，这里要自己起“站立”脚本
    startNormalAction(CharacterNormalAction::Idle);
}
