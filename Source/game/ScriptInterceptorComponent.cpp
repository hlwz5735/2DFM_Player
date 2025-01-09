//
// Created by limen on 2024/11/22.
//

#include "ScriptInterceptorComponent.hpp"
#include "2dfm/2dfmScriptItem.hpp"
#include "2dfm/CommonResource.hpp"
#include "MoveComponent.hpp"
#include "SeamlessScrollComponent.hpp"
#include "engine/AudioSystem.hpp"
#include "engine/KgtNode.hpp"
#include "engine/MathUtils.hpp"
#include "engine/SoundClip.hpp"

USING_NS_AX;

namespace {
uint8_t convertKgtRgbColorValue(byte rawByte) {
    const int8_t val = *(reinterpret_cast<char *>(&rawByte));
    return static_cast<uint8_t>(val + 32);
}

uint8_t convertKgtOpacityColorValue(byte rawByte) {
    const auto val = 32.f - static_cast<int>(rawByte);
    return static_cast<uint8_t>((val / 32.f) * 255);
}
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

void ScriptInterceptorComponent::initRunningScript(int scriptIdx) {
    if (runningStack.empty()) {
        runningStack.emplace_back(ScriptRunningInfo{
            .originalScriptIdx = scriptIdx,
            .originalOffset = 0,
        });
    }
    this->originalScriptIdx = scriptIdx;
    jumpToScriptItem(scriptIdx);
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
        AudioSystem::getInstance().playClip(soundClip, soundClip->isLoop(), 1.f);
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

_2dfm::ShowPic *ScriptInterceptorComponent::interceptScriptUntilShowPic() {
processHead:
    ++runningScriptItemIdx;

    while (runningScriptItemIdx < runningStack.back().scriptItemEndIdx) {
        const auto item = getCommonResource()->scriptItems[runningScriptItemIdx];
        const auto type = static_cast<_2dfm::CommonScriptItemTypes>(item->type);
        if (type == _2dfm::CommonScriptItemTypes::PIC) {
            return reinterpret_cast<_2dfm::ShowPic *>(item);
        }
        if (type == _2dfm::CommonScriptItemTypes::END) {
            spriteComponent->setTexture(nullptr);
            spriteComponent->setVisible(false);
            return nullptr;
        }
        if (type == _2dfm::CommonScriptItemTypes::SOUND) {
            interceptPlaySoundCmd(reinterpret_cast<_2dfm::PlaySoundCmd *>(item));
        } else if (type == _2dfm::CommonScriptItemTypes::COLOR) { // 色
            interceptColorSetCmd(reinterpret_cast<_2dfm::ColorSetCmd *>(item));
        } else if (type == _2dfm::CommonScriptItemTypes::MOVE) {
            interceptMoveCmd(reinterpret_cast<_2dfm::MoveCmd *>(item));
        } else if (type == _2dfm::CommonScriptItemTypes::JUMP) {
            auto jumpCmd = reinterpret_cast<_2dfm::JumpCmd *>(item);
            jumpToScriptItem(jumpCmd->jumpId, jumpCmd->jumpPos);
            continue;
        } else if (type == _2dfm::CommonScriptItemTypes::CALL) {
            auto callCmd = reinterpret_cast<_2dfm::JumpCmd *>(item);
            pushRunningScript(callCmd->jumpId, callCmd->jumpPos);
            continue;
        } else if (type == _2dfm::CommonScriptItemTypes::LOOP) {
            auto loopCmd = reinterpret_cast<_2dfm::LoopCmd *>(item);
            pushRunningScript(loopCmd->targetScriptId, loopCmd->targetPos, loopCmd->loopCount);
            continue;
        } else if (type == _2dfm::CommonScriptItemTypes::RANDOM) {
            auto randomCmd = reinterpret_cast<_2dfm::RandomCmd *>(item);
            auto hit = doRandom(randomCmd->randomMaxVal, randomCmd->moreThanVal);
            if (hit) {
                jumpToScriptItem(randomCmd->targetScriptId, randomCmd->targetPos);
                continue;
            }
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

    // 如果走到了最后，发现从头到尾都没有图片指令，则退出播放
    if (hasNoShowPicItem()) {
        spriteComponent->setVisible(false);
        return nullptr;
    } else {
        // 只要没遇到“完”，就会从头开始
        jumpToScriptItem(originalScriptIdx);
        goto processHead;
    }
}

void ScriptInterceptorComponent::interceptShowPicCmd(const _2dfm::ShowPic *cmd) {
    auto oldTex = spriteComponent->getTexture();
    auto tex = getCommonResource()->pictures.at(cmd->getPicIdx());
    if (tex) {
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
    auto seamlessComp = dynamic_cast<SeamlessScrollComponent *>(_owner->getComponent("SeamlessScrollComponent"));
    if (seamlessComp && oldTex != tex) {
        seamlessComp->updateSprite();
    }
}

void ScriptInterceptorComponent::interceptColorSetCmd(const _2dfm::ColorSetCmd *cmd) {
    auto setType = cmd->getColorBlendType();
    switch (setType) {
    case _2dfm::ColorBlendType::ALPHA_BLEND: {
        auto opacity = convertKgtOpacityColorValue(cmd->alpha);
        if (opacity == 0) {
            spriteComponent->setOpacity(0);
            spriteComponent->setVisible(false);
        } else {
            spriteComponent->setBlendFunc(BlendFunc::ALPHA_NON_PREMULTIPLIED);
            spriteComponent->setOpacity(opacity);
            spriteComponent->setVisible(true);
        }
    }
    break;
    case _2dfm::ColorBlendType::TRANSPARENCY:
        spriteComponent->setVisible(true);
        spriteComponent->setBlendFunc(BlendFunc::ALPHA_NON_PREMULTIPLIED);
        spriteComponent->setOpacity(128);
        break;
    case _2dfm::ColorBlendType::ADD_BLEND:
        spriteComponent->setVisible(true);
        spriteComponent->setBlendFunc(BlendFunc::ADDITIVE);
        spriteComponent->setOpacity(255);
        break;
    case _2dfm::ColorBlendType::MINUS_BLEND:
        spriteComponent->setVisible(true);
        spriteComponent->setBlendFunc({backend::BlendFactor::ONE, backend::BlendFactor::ONE_MINUS_SRC_COLOR});
        spriteComponent->setOpacity(255);
        break;
    case _2dfm::ColorBlendType::NORMAL:
    default:
        spriteComponent->setVisible(true);
        spriteComponent->setBlendFunc(BlendFunc::ALPHA_NON_PREMULTIPLIED);
        spriteComponent->setOpacity(255);
        break;
    }
    // TODO: 颜色叠加暂时关闭
    /*const ax::Color3B blendColor(convertKgtRgbColorValue(colorSet->red),
        convertKgtRgbColorValue(colorSet->green),
        convertKgtRgbColorValue(colorSet->blue));
    spriteComponent->setColor(blendColor);*/
}
