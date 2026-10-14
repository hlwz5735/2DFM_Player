//
// Created by limen on 2024/11/22.
//

#include "ScriptInterceptorComponent.hpp"

#include <algorithm>

#include "2dfm/2dfmScriptItem.hpp"
#include "2dfm/CommonResource.hpp"
#include "AfterimageComponent.hpp"
#include "AudioEngine.h"
#include "KgtColor.hpp"
#include "MoveComponent.hpp"
#include "engine/KgtNode.hpp"
#include "engine/MathUtils.hpp"
#include "engine/SoundClip.hpp"

USING_NS_AX;

namespace {
/// 等待时间的浮点误差阈值（秒）：小于它的正余量直接当成 0，见 update()
constexpr float kWaitEpsilon = 1e-4f;
} // namespace

namespace {
    // 颜色换算与混合方式解析见 KgtColor.hpp（与“残”共用）
}

bool ScriptInterceptorComponent::init() {
    if (!Component::init()) {
        return false;
    }
    return true;
}

void ScriptInterceptorComponent::onAdd() {
    Component::onAdd();
    spriteComponent = getOwner()->getSprite();
    moveComponent = getOwner()->getMoveComp();

    if (!spriteComponent || !moveComponent) {
        throw std::runtime_error("Can't get required components!");
    }
}

void ScriptInterceptorComponent::update(float deltaTime) {
    Component::update(deltaTime);
    if (!spriteComponent || getCommonResource() == nullptr) {
        return;
    }

    // 已经终止（遇到“完”/走到末尾的隐形“完”），或者正阻塞在一张“停留时间为 0”的图上：
    // 都不推进。注意后者是数据刻意安排的无限等待，与“等待时间减到 0 以下”完全是两回事
    // （“招”分歧就是用来打断它的，见 notifyMoveBranch）。
    if (timeWaiting == std::numeric_limits<float>::infinity() || runningStack.empty()
        || runningScriptItemIdx == runningStack.back().scriptItemEndIdx) {
        return;
    }

    if (timeWaiting > 0) {
        timeWaiting -= deltaTime;
        // 抹掉浮点误差：等待时间理论上“正好走完”时，减法可能残留一个极小的正数
        // （例如 6 × 1/60 与 0.1f 的误差约 1e-8），不抹掉的话这张图会白多显示一帧，
        // 短动画（几张图循环）就会被整体拖慢。阈值远小于一帧，不会造成“提前切换”。
        if (timeWaiting > 0 && timeWaiting < kWaitEpsilon) {
            timeWaiting = 0.f;
        }
    }
    onAdvanceTick(deltaTime);

    // ── 推进到“该显示哪张图” ──
    advanceUntilShowPic();
}

bool ScriptInterceptorComponent::advanceUntilShowPic() {
    // 这里同时承担了**跳帧补偿**：
    //  · 每张图的等待时间用 “+=” 累加，所以本帧多走掉的时间会以负余量的形式带到下一张图上，
    //    不会因为帧率波动而把整个脚本越拖越慢；
    //  · 用 while（而不是 if）连续推进，所以余量还 ≤ 0 时，中间那些图**一帧都不显示**就被跳过，
    //    期间遇到的“声 / 色 / 移 / 跳 / 调 / 循 / 随 / 物 / 残”等脚本项照常执行；
    //  · 回头循环（没遇到“完”）、状态机接上的下一段脚本，也都在这同一个 while 里继续吃余量，
    //    所以它们同样享受这套补偿。
    const _2dfm::ShowPic *showPicScript = nullptr;
    while (timeWaiting <= 0) {
        showPicScript = interceptScriptUntilShowPic();
        if (!showPicScript) {
            timeWaiting = std::numeric_limits<float>::infinity();
            runningScriptItemIdx = runningStack.back().scriptItemEndIdx;
            return false;
        }
        float keepTime = static_cast<float>(showPicScript->keepTime) / 100;
        if (keepTime == 0) {
            // “图”自身设定为 0：这张图无限等待，脚本执行就阻塞在这里
            // （由“招”脚本按别的条件介入打断，见 notifyMoveBranch）
            timeWaiting = std::numeric_limits<float>::infinity();
        } else {
            timeWaiting += keepTime;
        }
    }
    if (showPicScript) {
        this->interceptShowPicCmd(showPicScript);
    }
    return true;
}

void ScriptInterceptorComponent::initRunningScript(int scriptIdx, int offset) {
    if (!runningStack.empty()) {
        runningStack.clear();
    }
    // 新一轮执行：上一轮留下的“招”监听一律作废。
    // 外部原因强制换脚本（受伤、满足指令、状态机接管下一段）都会走这里，所以监听随之移除；
    // 而脚本内部的“跳 / 调 / 循 / 随”不经过这里，监听继续有效（范围见 MoveBranchCmd 的说明）。
    clearMoveBranchListeners();
    runningStack.emplace_back(ScriptRunningInfo{
        .originalScriptIdx = scriptIdx,
        .originalOffset = offset,
    });
    this->originalScriptIdx = scriptIdx;
    this->scriptFinished = false;
    // 立即跳转
    this->timeWaiting = 0;
    jumpToScriptItem(scriptIdx, offset);
}

void ScriptInterceptorComponent::jumpToScriptItem(int scriptIdx, int offset) {
    auto &script = getCommonResource()->scripts.at(scriptIdx);
    auto &back = runningStack.back();

    back.scriptItemStartIdx = script.startIdx;
    back.scriptItemEndIdx = script.endIdx;
    back.stashedScriptItemIdx = script.startIdx + offset;

    // 设置当前运行脚本的索引
    this->runningScriptItemIdx = back.stashedScriptItemIdx;
}

void ScriptInterceptorComponent::pushRunningScript(int scriptIdx, int offset, int loop) {
    if (loop == 0) {
        return;
    }
    if (runningStack.size() > 255) {
        throw std::runtime_error("Script stack overflow");
    }
    auto &tail = runningStack.back();
    // 暂存当前运行的脚本下标
    tail.stashedScriptItemIdx = this->runningScriptItemIdx;

    runningStack.emplace_back(ScriptRunningInfo{
        .originalScriptIdx = scriptIdx,
        .originalOffset = offset,
        .loopCount = loop,
    });
    jumpToScriptItem(scriptIdx, offset);
}

void ScriptInterceptorComponent::interceptPlaySoundCmd(const _2dfm::PlaySoundCmd *cmd) {
    auto soundClip = getCommonResource()->sounds.at(cmd->soundIdx);
    if (soundClip) {
        ax::AudioEngine::play2d(soundClip->getVirtualFilePath(), soundClip->isLoop(), 1.f);
    }
}

void ScriptInterceptorComponent::stop() {
    this->timeWaiting = std::numeric_limits<float>::infinity();
    this->runningStack.clear();
    // 停止后既没有脚本在播，也谈不上“播完”
    this->scriptFinished = true;
    clearMoveBranchListeners();
    // 停止解析执行时一并回收残影，避免角色节点消失后留下孤儿精灵
    if (auto *owner = getOwner(); owner != nullptr && owner->getAfterimageComp() != nullptr) {
        owner->getAfterimageComp()->clear();
    }
}

bool ScriptInterceptorComponent::hasNoShowPicItem() const {
    auto commonResource = getCommonResource();
    auto &originalScript = commonResource->scripts.at(originalScriptIdx);

    for (int i = originalScript.startIdx; i < originalScript.endIdx; ++i) {
        auto item = commonResource->scriptItems[i];
        if (static_cast<_2dfm::CommonScriptItemTypes>(item->type) == _2dfm::CommonScriptItemTypes::SHOW_PIC) {
            return false;
        }
    }
    return true;
}

void ScriptInterceptorComponent::interceptMoveCmd(const _2dfm::MoveCmd *moveCmd) const {
    auto vel = ax::Vec2(moveCmd->moveX, moveCmd->moveY) * 0.01f;
    auto accel = ax::Vec2(moveCmd->accelX, moveCmd->accelY) * 0.01f;
    auto &oriVel = moveComponent->getVelocity();
    auto &oriAccel = moveComponent->getAcceleration();
    if (moveCmd->isAdd()) {
        // 相加模式
        vel.x += (moveCmd->isIgnoreMoveX() ? 0 : oriVel.x);
        vel.y += (moveCmd->isIgnoreMoveY() ? 0 : oriVel.y);
        accel.x += (moveCmd->isIgnoreAccelX() ? 0 : oriAccel.x);
        accel.y += (moveCmd->isIgnoreAccelY() ? 0 : oriAccel.y);
    } else {
        // 代入模式
        vel.x = moveCmd->isIgnoreMoveX() ? oriVel.x : vel.x;
        vel.y = moveCmd->isIgnoreMoveY() ? oriVel.y : vel.y;
        accel.x = moveCmd->isIgnoreAccelX() ? oriAccel.x : accel.x;
        accel.y = moveCmd->isIgnoreAccelY() ? oriAccel.y : accel.y;
    }
    moveComponent->setAcceleration(accel);
    moveComponent->setVelocity(vel);
}

void ScriptInterceptorComponent::interceptObjectCmd(const _2dfm::ObjectCmd *cmd) const {
    // 暂时不做实现
}

void ScriptInterceptorComponent::interceptAfterimageCmd(const _2dfm::AfterimageCmd *cmd) {
    auto *owner = getOwner();
    if (owner == nullptr || owner->getAfterimageComp() == nullptr) {
        return;
    }
    owner->getAfterimageComp()->applyCommand(cmd);
}

void ScriptInterceptorComponent::interceptMoveBranchCmd(const _2dfm::MoveBranchCmd *cmd) {
    const auto cond = cmd->getCondition();
    if (cond == _2dfm::MoveBranchCmd::Cond::NONE) {
        // “无”：不监听任何条件
        return;
    }
    // 数据自检：目标脚本要存在，偏移不能越过该脚本末尾
    // （偏移恰等于脚本长度是合法的，含义是直接跳到该段末尾 —— 真实数据里就有这种写法）
    const auto *resource = getCommonResource();
    if (resource == nullptr || static_cast<size_t>(cmd->targetScriptId) >= resource->scripts.size()) {
        AXLOGW("ScriptInterceptorComponent: “招”的条件({}) 目标脚本 ID({}) 不存在，忽略该监听",
               static_cast<int>(cond), cmd->targetScriptId);
        return;
    }
    const auto &target = resource->scripts[cmd->targetScriptId];
    if (static_cast<int>(cmd->targetPos) > target.endIdx - target.startIdx) {
        AXLOGW("ScriptInterceptorComponent: “招”的条件({}) 目标偏移({}) 超出脚本 {} 的范围，忽略该监听",
               static_cast<int>(cond), cmd->targetPos, cmd->targetScriptId);
        return;
    }
    // 同一个条件只留一条：脚本回头循环又执行到这个脚本项时按“重新设置”处理，不堆叠监听
    for (auto &listener : moveBranchListeners) {
        if (listener.condition == cond) {
            listener.targetScriptId = cmd->targetScriptId;
            listener.targetPos = cmd->targetPos;
            return;
        }
    }
    moveBranchListeners.push_back(MoveBranchListener{cond, cmd->targetScriptId, cmd->targetPos});
    AXLOGI("ScriptInterceptorComponent: 登记“招”监听 条件={} → 脚本 {} 偏移 {}", static_cast<int>(cond),
           cmd->targetScriptId, cmd->targetPos);
}

void ScriptInterceptorComponent::clearMoveBranchListeners() {
    moveBranchListeners.clear();
}

bool ScriptInterceptorComponent::finishScriptExecution(bool allowHandoff) {
    this->scriptFinished = true;
    // “完”（显式的，或者“已着陆仍向下移动”这种强制结束）意味着“招”监听到此为止
    clearMoveBranchListeners();
    if (allowHandoff && continueWithNextScript()) {
        return true;
    }
    this->clearSpriteOnScriptEnd();
    this->onScriptStopped();
    // 本次执行到此为止：后面不会再推进这张图
    this->timeWaiting = std::numeric_limits<float>::infinity();
    if (!runningStack.empty()) {
        this->runningScriptItemIdx = runningStack.back().scriptItemEndIdx;
    }
    return false;
}

bool ScriptInterceptorComponent::shouldFinishAtGround() const {
    if (moveComponent == nullptr) {
        return false;
    }
    // 实际速度仍然朝下（逻辑坐标 Y 向下为正）：移动还在把角色往地面里压。
    // “移”是继承的，所以这个速度只可能来自脚本自己下的“移”。
    return moveComponent->getVelocity().y > 0;
}

void ScriptInterceptorComponent::notifyLanded() {
    // 没有脚本在播（或者本次执行已经终止）就没什么可判的
    if (scriptFinished || runningStack.empty()) {
        return;
    }
    // 1) 脚本登记过“招：着地后” → 跳转到它指定的位置，并清掉这条监听。
    //    注意这是“落地”这一下的事件：角色本来就站在地上时，登记了也不会立刻生效
    //    （要等真的跳起来再落地），与 2DFM 的实际行为一致。
    if (notifyMoveBranch(_2dfm::MoveBranchCmd::Cond::LANDED)) {
        return;
    }
    // 2) 没有着地监听：只有实际速度还朝下才算“移动撞地到头了”，按“完”处理；
    //    脚本自己已经把 Y 速度停住（归零 / 向上）的话，这一段继续播下去。
    if (shouldFinishAtGround()) {
        AXLOGI("ScriptInterceptorComponent: 已着陆速度仍向下（y={}），本次执行结束",
               moveComponent->getVelocity().y);
        if (finishScriptExecution()) {
            // 上层（状态机）给了下一段脚本：当帧继续解释出图，不白停一帧
            advanceUntilShowPic();
        }
    }
}

bool ScriptInterceptorComponent::notifyMoveBranch(_2dfm::MoveBranchCmd::Cond cond) {
    if (cond == _2dfm::MoveBranchCmd::Cond::NONE || runningStack.empty()) {
        return false;
    }
    const auto listener = std::find_if(moveBranchListeners.begin(), moveBranchListeners.end(),
                                       [cond](const MoveBranchListener &l) { return l.condition == cond; });
    if (listener == moveBranchListeners.end()) {
        return false;
    }
    const int targetScriptId = listener->targetScriptId;
    const int targetPos = listener->targetPos;
    // 命中即消耗：同一个条件不再重复触发，要再触发得由脚本重新登记
    moveBranchListeners.erase(listener);

    AXLOGI("ScriptInterceptorComponent: “招”条件（{}）满足 → 跳转 脚本 {} 偏移 {}", static_cast<int>(cond),
           targetScriptId, targetPos);

    jumpToScriptItem(targetScriptId, targetPos);
    // 解除“停留时间为 0 的图”造成的无限等待（这时 timeWaiting 是 infinity），
    // 并立刻解释到新脚本的第一张图，做到当帧生效
    timeWaiting = 0.f;
    advanceUntilShowPic();
    return true;
}

_2dfm::ShowPic *ScriptInterceptorComponent::interceptScriptUntilShowPic() {
    // 进入解释执行就说明脚本还在播
    this->scriptFinished = false;
    // 添加一个跳转计数，如果都跳转了255次还没遇到“图”或者“停”，那就别扯犊子了，退出解释执行
    unsigned jumpCount = 0;
    // 同一个调用里只允许无缝接下一段脚本一次：
    // 万一接上的是一段没有任何“图”的脚本（跑完就到底），双方会互相接力形成死循环
    bool continued = false;
processHead:
    ++runningScriptItemIdx;
processItem:
    while (runningScriptItemIdx < runningStack.back().scriptItemEndIdx && jumpCount < 255) {
        const auto item = getCommonResource()->scriptItems[runningScriptItemIdx];
        const auto type = static_cast<_2dfm::CommonScriptItemTypes>(item->type);
        if (type == _2dfm::CommonScriptItemTypes::SHOW_PIC) { // 图
            return reinterpret_cast<_2dfm::ShowPic *>(item);
        }
        if (type == _2dfm::CommonScriptItemTypes::END) { // 完
            // “完”只代表这一段执行完了：先问上层（Player 的状态机）接下来播什么，
            // 给了就同一帧无缝接上，没给才收尾。
            // 本次调用已经无缝接过一段时不再要下一段：互相接力会形成死循环（见 continued）。
            if (finishScriptExecution(!continued)) {
                continued = true;
                jumpCount = 0;
                goto processItem;
            }
            return nullptr;
        }
        if (type == _2dfm::CommonScriptItemTypes::PLAY_SOUND) { // 声
            interceptPlaySoundCmd(reinterpret_cast<_2dfm::PlaySoundCmd *>(item));
        } else if (type == _2dfm::CommonScriptItemTypes::COLOR) { // 色
            interceptColorSetCmd(reinterpret_cast<_2dfm::ColorSetCmd *>(item));
        } else if (type == _2dfm::CommonScriptItemTypes::MOVE) { // 移
            interceptMoveCmd(reinterpret_cast<_2dfm::MoveCmd *>(item));
        } else if (type == _2dfm::CommonScriptItemTypes::JUMP) { // 跳
            auto jumpCmd = reinterpret_cast<_2dfm::JumpCmd *>(item);
            jumpToScriptItem(jumpCmd->jumpId, jumpCmd->jumpPos);
            onScriptJumped();
            ++jumpCount;
            continue;
        } else if (type == _2dfm::CommonScriptItemTypes::CALL) { // 调
            auto callCmd = reinterpret_cast<_2dfm::JumpCmd *>(item);
            pushRunningScript(callCmd->jumpId, callCmd->jumpPos);
            onScriptJumped();
            continue;
        } else if (type == _2dfm::CommonScriptItemTypes::LOOP) { // 循
            auto loopCmd = reinterpret_cast<_2dfm::LoopCmd *>(item);
            pushRunningScript(loopCmd->targetScriptId, loopCmd->targetPos, loopCmd->loopCount);
            onScriptJumped();
            continue;
        } else if (type == _2dfm::CommonScriptItemTypes::RANDOM_BRANCH) { // 随
            auto randomCmd = reinterpret_cast<_2dfm::RandomCmd *>(item);
            auto hit = doRandom(randomCmd->randomMaxVal, randomCmd->moreThanVal);
            if (hit) {
                jumpToScriptItem(randomCmd->targetScriptId, randomCmd->targetPos);
                onScriptJumped();
                continue;
            }
        } else if (type == _2dfm::CommonScriptItemTypes::SPAWN_OBJECT) { // 物
            interceptObjectCmd(reinterpret_cast<_2dfm::ObjectCmd *>(item));
        } else if (type == _2dfm::CommonScriptItemTypes::AFTERIMAGE) { // 残
            interceptAfterimageCmd(reinterpret_cast<_2dfm::AfterimageCmd *>(item));
        } else if (type == _2dfm::CommonScriptItemTypes::MOVE_BRANCH) { // 招
            interceptMoveBranchCmd(reinterpret_cast<const _2dfm::MoveBranchCmd *>(item));
        }

        // 程序计数器向后+1
        ++runningScriptItemIdx;
    }

    // 如果走到了最后，且目前调用栈仍有子调用
    if (runningStack.size() > 1) {
        auto &back = runningStack.back();
        // 处理循环问题
        if (--back.loopCount > 0) {
            jumpToScriptItem(back.originalScriptIdx, back.originalOffset);
        } else {
            // 无循环，弹出调用栈，解释指针指回上一个调用点
            runningStack.pop_back();
            runningScriptItemIdx = runningStack.back().stashedScriptItemIdx;
        }
        goto processHead;
    }

    // 走到这里说明最外层脚本已经播到末尾（没有遇到“完”）。
    // 场景 / KGT / DEMO：回到开头继续循环；Player：末尾相当于有一个隐形的“完”。
    if (shouldLoopAtScriptEnd()) {
        // 整段脚本一个“图”都没有的话，回头循环只会在这次调用里空转，直接结束
        if (hasNoShowPicItem()) {
            this->scriptFinished = true;
            clearMoveBranchListeners();
            this->clearSpriteOnScriptEnd();
            this->onScriptStopped();
            return nullptr;
        }
        jumpToScriptItem(originalScriptIdx);
        goto processHead;
    }

    // Player：这一段执行完了（末尾相当于隐形的“完”），同样先问上层接下来播什么，
    // 给了就同一帧无缝接上；监听也随这次执行一起清空。
    if (finishScriptExecution(!continued)) {
        continued = true;
        jumpCount = 0;
        goto processItem;
    }
    return nullptr;
}

bool ScriptInterceptorComponent::continueWithNextScript() {
    const int nextScriptId = pickNextScriptAfterFinish();
    if (nextScriptId < 0) {
        return false;
    }
    const auto *resource = getCommonResource();
    if (resource == nullptr || nextScriptId >= static_cast<int>(resource->scripts.size())) {
        AXLOGW("ScriptInterceptorComponent: 上层给出的下一段脚本 ID({}) 不合法", nextScriptId);
        return false;
    }
    // 重置执行栈并从新脚本的第一项开始接着解释：同一个调用里就会返回它的“图”，
    // 也就是下一段脚本当帧生效。
    initRunningScript(nextScriptId);
    return true;
}

void ScriptInterceptorComponent::clearSpriteOnScriptEnd() {
    if (!shouldClearSpriteOnScriptEnd()) {
        return;
    }
    spriteComponent->setTexture(nullptr);
    spriteComponent->setVisible(false);
}

void ScriptInterceptorComponent::interceptShowPicCmd(const _2dfm::ShowPic *cmd) {
    const auto tex = getCommonResource()->pictures.at(cmd->getPicIdx());
    if (tex && tex->getContentSizeInPixels().width > 0 && tex->getContentSizeInPixels().height > 0) {
        const auto blendFunc = spriteComponent->getBlendFunc();
        const auto visible = spriteComponent->isVisible();
        spriteComponent->setTexture(tex);
        const auto &r = tex->getContentSizeInPixels();
        spriteComponent->setTextureRect({0, 0, r.width, r.height});
        spriteComponent->setPosition(cmd->getOffset().x, -cmd->getOffset().y);
        spriteComponent->setVisible(visible);
        spriteComponent->setBlendFunc(blendFunc);
        spriteComponent->setFlippedX(cmd->isFlipX());
        spriteComponent->setFlippedY(cmd->isFlipY());
    } else {
        spriteComponent->setTexture(nullptr);
        spriteComponent->setVisible(false);
    }
}

void ScriptInterceptorComponent::interceptColorSetCmd(const _2dfm::ColorSetCmd *cmd) {
    const auto setType = cmd->getColorBlendType();
    const auto opacity = convertKgtOpacityColorValue(cmd->alpha);
    const auto blend = resolveKgtColorBlend(setType, opacity);

    if (setType == _2dfm::ColorBlendType::ALPHA_BLEND && opacity == 0) {
        spriteComponent->setOpacity(0);
        spriteComponent->setVisible(false);
    } else {
        spriteComponent->setVisible(true);
        spriteComponent->setBlendFunc(blend.blendFunc);
        spriteComponent->setOpacity(blend.baseOpacity);
    }

    float colorArr[3] = {
        convertKgtRgbColorValue(cmd->red),
        convertKgtRgbColorValue(cmd->green),
        convertKgtRgbColorValue(cmd->blue)
    };
    auto ps = spriteComponent->getProgramState();
    auto loc = ps->getUniformLocation("_2dfmColor");
    spriteComponent->getProgramState()->setUniform(loc, &colorArr[0], sizeof(colorArr));
}
