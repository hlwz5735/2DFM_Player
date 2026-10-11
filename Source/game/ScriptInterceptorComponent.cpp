//
// Created by limen on 2024/11/22.
//

#include "ScriptInterceptorComponent.hpp"

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
    // 都不推进。注意后者是数据刻意安排的无限等待，与“等待时间减到 0 以下”完全是两回事。
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
            return;
        }
        float keepTime = static_cast<float>(showPicScript->keepTime) / 100;
        if (keepTime == 0) {
            // “图”自身设定为 0：这张图无限等待，脚本执行就阻塞在这里
            // （以后由“招”脚本按别的条件介入打断）
            timeWaiting = std::numeric_limits<float>::infinity();
        } else {
            timeWaiting += keepTime;
        }
    }
    if (showPicScript) {
        this->interceptShowPicCmd(showPicScript);
    }
}

void ScriptInterceptorComponent::initRunningScript(int scriptIdx, int offset) {
    if (!runningStack.empty()) {
        runningStack.clear();
    }
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
        if (static_cast<_2dfm::CommonScriptItemTypes>(item->type) == _2dfm::CommonScriptItemTypes::PIC) {
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
        if (type == _2dfm::CommonScriptItemTypes::PIC) { // 图
            return reinterpret_cast<_2dfm::ShowPic *>(item);
        }
        if (type == _2dfm::CommonScriptItemTypes::END) { // 完
            // “完”只代表这一段执行完了：先问上层（Player 的状态机）接下来播什么，
            // 给了就同一帧无缝接上，没给才收尾。
            this->scriptFinished = true;
            if (!continued && continueWithNextScript()) {
                continued = true;
                jumpCount = 0;
                goto processItem;
            }
            this->clearSpriteOnScriptEnd();
            this->onScriptStopped();
            return nullptr;
        }
        if (type == _2dfm::CommonScriptItemTypes::SOUND) { // 声
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
        } else if (type == _2dfm::CommonScriptItemTypes::RANDOM) { // 随
            auto randomCmd = reinterpret_cast<_2dfm::RandomCmd *>(item);
            auto hit = doRandom(randomCmd->randomMaxVal, randomCmd->moreThanVal);
            if (hit) {
                jumpToScriptItem(randomCmd->targetScriptId, randomCmd->targetPos);
                onScriptJumped();
                continue;
            }
        } else if (type == _2dfm::CommonScriptItemTypes::OBJECT) { // 物
            interceptObjectCmd(reinterpret_cast<_2dfm::ObjectCmd *>(item));
        } else if (type == _2dfm::CommonScriptItemTypes::AFTERIMAGE) { // 残
            interceptAfterimageCmd(reinterpret_cast<_2dfm::AfterimageCmd *>(item));
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
            this->clearSpriteOnScriptEnd();
            this->onScriptStopped();
            return nullptr;
        }
        jumpToScriptItem(originalScriptIdx);
        goto processHead;
    }

    // Player：这一段执行完了，同样先问上层接下来播什么，给了就同一帧无缝接上
    this->scriptFinished = true;
    if (!continued && continueWithNextScript()) {
        continued = true;
        jumpCount = 0;
        goto processItem;
    }
    this->clearSpriteOnScriptEnd();
    this->onScriptStopped();
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
