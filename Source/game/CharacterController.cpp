//
// CharacterController: 角色控制器（状态机）
//
// 详细的状态说明见 CharacterController.hpp。
//

#include "CharacterController.hpp"

#include <cmath>

#include "2dfm/2dfmScriptItem.hpp"
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
    this->playerData = data;
    this->commandMatcher.setPlayerData(std::move(data));
}

const char *CharacterController::getAttackCommandName() const {
    if (poseState != CharacterPoseState::Attack || attackCommand == nullptr) {
        return nullptr;
    }
    return attackCommand->name.c_str();
}

void CharacterController::enterAttackByScriptJump() {
    // 通常状态的脚本（站立、行走、下蹲……）一旦跳转到别的脚本，就当作进入了「攻击」：
    // 在它执行完之前不再响应输入；播完后由结束回调按“此刻的位置状态 + 输入”交还给通常。
    // 浮空/下蹲时同理，位置状态不动，只改姿势。
    if (poseState != CharacterPoseState::Normal) {
        return; // 已经在攻击里了（或以后别的姿势），不用重复处理
    }
    poseState = CharacterPoseState::Attack;
    attackCommand = nullptr; // 不是由出招指令发动的，没有招式名
}

void CharacterController::logUsableCommands() const {
    if (playerData == nullptr) {
        return;
    }
    int usable = 0;
    for (const auto &command : playerData->commands) {
        if (command.steps.size() != 1) {
            continue;
        }
        const auto &step = command.steps.front();
        if (step.mark != CommandStepMark::NORMAL || step.buttons == 0) {
            continue;
        }
        ++usable;
        AXLOGI("[CharacterController] 可用指令「{}」方向={} 按键=0x{:02X} 分支：空中={} 站立远距={} 下蹲={}（站立近距={}，"
               "测试场景没有第二个角色，暂不参与）",
               command.name, static_cast<int>(step.direction), step.buttons,
               command.airScriptId, command.standFarScriptId, command.crouchScriptId, command.standNearScriptId);
    }
    AXLOGI("[CharacterController] 出招指令共 {} 条，其中本版可用的单指令项 {} 条",
           playerData->commands.size(), usable);
}

void CharacterController::logNormalScriptJumps() const {
    if (playerData == nullptr) {
        return;
    }
    struct NamedScript {
        const char *label;
        int scriptId;
    };
    const NamedScript normalScripts[] = {
        {"站立", playerData->standScriptId},
        {"前行", playerData->forwardScriptId},
        {"后退", playerData->backwardScriptId},
        {"下蹲", playerData->crouchScriptId},
        {"蹲着", playerData->crouchingScriptId},
        {"从下蹲站起", playerData->standUpScriptId},
        {"空中落下", playerData->fallingScriptId},
    };

    const int scriptCount = static_cast<int>(playerData->scripts.size());
    for (const auto &named : normalScripts) {
        if (named.scriptId <= 0 || named.scriptId >= scriptCount) {
            continue;
        }
        const auto &script = playerData->scripts[named.scriptId];
        std::string found;
        for (int i = script.startIdx; i < script.endIdx; ++i) {
            const auto type = static_cast<_2dfm::CommonScriptItemTypes>(playerData->scriptItems[i]->type);
            const char *label = nullptr;
            switch (type) {
            case _2dfm::CommonScriptItemTypes::JUMP:
                label = "跳";
                break;
            case _2dfm::CommonScriptItemTypes::CALL:
                label = "调";
                break;
            case _2dfm::CommonScriptItemTypes::LOOP:
                label = "循";
                break;
            case _2dfm::CommonScriptItemTypes::RANDOM:
                label = "随";
                break;
            default:
                break;
            }
            if (label != nullptr) {
                if (!found.empty()) {
                    found += "、";
                }
                found += label;
            }
        }
        if (!found.empty()) {
            // 提醒：这类脚本一开播就会把角色带进「攻击」状态（不再响应输入），
            // 只有在确实想把执行交给别的脚本时才该这么写
            AXLOGW("[CharacterController] 通常状态的「{}」脚本（id={}）里含 {} —— 该脚本播放期间会按攻击状态处理，不响应输入",
                   named.label, named.scriptId, found);
        }
    }
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
        // 攻击姿势由出招指令发动；挨打 / 防御需要中招判定与防御按键，都还没做
        AXLOGW("CharacterController: 姿势状态 {} 不能这样直接切换（攻击由指令发动，挨打/防御尚未实现）",
               toString(pose));
        return;
    }
    poseState = pose;
    attackCommand = nullptr;
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
        // 出招指令与通常脚本的数据自检：列出本版能用的指令、以及会被当作攻击的通常脚本
        logUsableCommands();
        logNormalScriptJumps();
        // 默认状态：站立 + 通常，反复播放“站立”脚本
        startNormalAction(CharacterNormalAction::Idle);
    }

    // 浮空的触地判定与姿势无关：空中攻击 / 空中挨打一样会落地
    if (positionState == CharacterPositionState::Airborne) {
        updateAirborne(delta);
    }

    switch (poseState) {
    case CharacterPoseState::Normal:
        updateNormalPose(delta);
        break;
    case CharacterPoseState::Attack:
        updateAttackPose(delta);
        break;
    default:
        // 挨打 / 防御：见 switchPose()
        break;
    }
}

// ─────────────────────────── 脚本结束回调（由解释器驱动） ───────────────────────────

int CharacterController::pickNextScriptAfterFinish() {
    switch (poseState) {
    case CharacterPoseState::Normal:
        return pickNextNormalScriptAfterFinish();
    case CharacterPoseState::Attack:
        return pickNextScriptAfterAttackFinish();
    default:
        // 挨打 / 防御：见 switchPose()
        return -1;
    }
}

int CharacterController::pickNextNormalScriptAfterFinish() {
    switch (normalAction) {
    case CharacterNormalAction::CrouchDown:
        // “下蹲”演完，蹲着开始：这时才算真的蹲下来了
        positionState = CharacterPositionState::Crouch;
        return decideNormalAction(CharacterNormalAction::CrouchHold);

    case CharacterNormalAction::StandUp:
        // “从下蹲站起”演完，站立待机开始：这时才算真的站好了
        positionState = CharacterPositionState::Stand;
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
            enterAirborne();
            return decideNormalAction(CharacterNormalAction::Falling);
        }
        resetAirborneOnLanding();
        return decideNormalAction(CharacterNormalAction::Idle);
    }
    return -1;
}

int CharacterController::pickNextScriptAfterAttackFinish() {
    // 攻击脚本执行完（含跳/调/循转到别处后走到终点）→ 交还给通常状态。
    // 按“此刻”的位置状态与输入决定下一段：站立攻击恢复时按着下就是走下蹲过渡、
    // 下蹲攻击恢复时松开下就是走下蹲站起、按着上就是转跳跃，与平时通常状态的转换完全一致。
    poseState = CharacterPoseState::Normal;
    attackCommand = nullptr;

    if (positionState == CharacterPositionState::Airborne) {
        // 打完还在空中：空中通常状态，播“空中落下”
        return decideNormalAction(CharacterNormalAction::Falling);
    }
    return decideNormalAction(decideNormalActionFromInput());
}

// ─────────────────────────── 每帧：输入 / 升空 / 触地 / 指令 ───────────────────────────

void CharacterController::updateNormalPose(float delta) {
    // 过渡动作（下蹲 / 从下蹲站起）期间不响应输入，推进完全由解释器的脚本结束回调负责
    if (isTransitionAction(normalAction)) {
        return;
    }

    // 先查出招指令：命中就发动攻击，指令优先于走 / 蹲 / 跳。
    // 这一步必须放在“浮空直接 return”之前 —— 空中拳脚走的就是 air 分支。
    if (tryStartAttackByCommand()) {
        return;
    }

    // 浮空中：方向键一律不响应（触地判定上面已经做过）
    if (positionState == CharacterPositionState::Airborne) {
        return;
    }
    // 起跳阶段：等角色真的被脚本带起来（脚本播完的收尾在结束回调里）
    if (normalAction == CharacterNormalAction::JumpUp) {
        if (isLiftingOff()) {
            enterAirborne();
        }
        return;
    }

    // 没有命中指令：按输入决定通常动作
    const auto decided = decideNormalActionFromInput();
    if (decided != normalAction) {
        startNormalAction(decided);
    }
}

void CharacterController::updateAttackPose(float delta) {
    // 攻击姿势下不响应任何输入，必须等脚本执行完毕，再由结束回调交还给通常状态。
    // TODO: 脚本里出现“动”“令”脚本项时，要按那两个脚本项的条件选择性放行输入。
}

void CharacterController::updateAirborne(float delta) {
    auto *node = getPlayerNode();
    const float y = node->getLogicPosition().y;

    // leftGround 只看“位置真的升高过”，免得刚起跳（人还在原地、只有向上速度）就判成落地
    if (y < getGroundY() - kLiftoffEpsilon) {
        leftGround = true;
    }
    if (!leftGround || y < getGroundY()) {
        return;
    }

    // 触地：位置状态无条件回站立
    if (poseState == CharacterPoseState::Normal) {
        // 通常姿势下落地就直接接“站立待机”
        land();
    } else {
        // 攻击姿势下只是贴地回站立，脚本继续播（空中招落地不打断），
        // 等它执行完由结束回调按“此刻是站立”来交还
        resetAirborneOnLanding();
    }
}

CharacterNormalAction CharacterController::decideNormalActionFromInput() {
    const auto dpad = Input::getInstance().getDPad();
    const bool down = Input::isDownDirection(dpad);
    const bool up = Input::isUpDirection(dpad);

    // 与跳跃一致：过渡脚本播放期间仍算“原来那个状态”。
    // 下蹲脚本演完（转 蹲着）才算真的蹲下，站起脚本演完（转 站立待机）才算真的站好。
    if (positionState == CharacterPositionState::Crouch) {
        if (down) {
            // 持续按住下方向（不论是否有左、右）：持续播放“蹲着”
            // TODO: 蹲着前行 / 蹲着后退（crouchMoveForward/BackwardScriptId）本版按需求先不接
            return CharacterNormalAction::CrouchHold;
        }
        if (!up) {
            // 松开下方向、也没按上：从下蹲站起（位置状态要等站起脚本演完才变）
            return CharacterNormalAction::StandUp;
        }
        // 按着上方向：直接转跳跃（下蹲攻击恢复时按着上就是这条路）
    } else {
        if (down) {
            // 站立 → 下蹲：先播“下蹲”过渡脚本，位置状态等它演完（转 蹲着）才变成下蹲
            return CharacterNormalAction::CrouchDown;
        }
        if (!up) {
            // 行走：角色面向右时，右=前行，左=后退
            // TODO: 角色转向（站立转身 / 蹲着转身）尚未实现，这里先固定按“面向右”解释
            if (dpad == Input::DPad::Right) {
                return CharacterNormalAction::WalkForward;
            }
            if (dpad == Input::DPad::Left) {
                return CharacterNormalAction::WalkBackward;
            }
            return CharacterNormalAction::Idle;
        }
        // 按着上方向：转跳跃（下面统一处理）
    }

    // 上类方向“按下或按住”都可以起跳：按住不放时每次回到站立都会再跳，这就是连跳。
    // 前跳 / 后跳脚本一开播就升空；上跳则要等角色真被脚本带起来（见 updateNormalPose）。
    switch (dpad) {
    case Input::DPad::RightUp:
        enterAirborne();
        return CharacterNormalAction::JumpForward;
    case Input::DPad::LeftUp:
        enterAirborne();
        return CharacterNormalAction::JumpBackward;
    default:
        return CharacterNormalAction::JumpUp;
    }
}

// ─────────────────────────── 出招指令 ───────────────────────────

bool CharacterController::canStartAttackByCommand() const {
    // 过渡动作（下蹲 / 从下蹲站起）期间不接受指令，等它演完
    if (isTransitionAction(normalAction)) {
        return false;
    }
    // 起跳阶段（还站在地上的上跳）不接受指令，升空之后才接空中指令
    if (positionState == CharacterPositionState::Stand && normalAction == CharacterNormalAction::JumpUp) {
        return false;
    }
    return true;
}

int CharacterController::scriptIdForCommandState(const PlayerCommand &command) const {
    int scriptId = -1;
    switch (positionState) {
    case CharacterPositionState::Airborne:
        scriptId = command.airScriptId;
        break;
    case CharacterPositionState::Crouch:
        scriptId = command.crouchScriptId;
        break;
    case CharacterPositionState::Stand:
        // 测试场景里没有第二个角色，距离永远算“远距”，所以用站立远距分支；
        // 站立近距分支不参与（某个分支没设置时，这条指令在该状态就什么都不做）
        scriptId = command.standFarScriptId;
        break;
    }
    if (scriptId <= 0 || scriptId >= static_cast<int>(playerData->scripts.size())) {
        return -1;
    }
    return scriptId;
}

bool CharacterController::tryStartAttackByCommand() {
    if (!canStartAttackByCommand()) {
        return false;
    }
    const auto match = commandMatcher.match();
    if (!match.hit()) {
        return false;
    }
    // 分支按当前位置状态挑：下蹲过渡 / 站起过渡期间不会走到这里（那两个动作把输入屏蔽了），
    // 所以“位置状态”就是玩家此刻真正所处的形态。
    const int scriptId = scriptIdForCommandState(*match.command);
    if (scriptId <= 0) {
        // 指令条件满足、但当前状态的分支没有设置：按规则什么都不做；
        // 命中的是这条（优先级更高）指令，所以也不再往后看别的指令
        return false;
    }
    startAttack(*match.command, scriptId);
    return true;
}

void CharacterController::startAttack(const PlayerCommand &command, int scriptId) {
    poseState = CharacterPoseState::Attack;
    attackCommand = &command;
    playScript(scriptId);
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

void CharacterController::enterAirborne() {
    positionState = CharacterPositionState::Airborne;
    // 跳跃脚本继续播，“接下来播哪段”由调用方决定
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