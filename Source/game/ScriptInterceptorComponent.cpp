//
// Created by limen on 2024/11/22.
//

#include "ScriptInterceptorComponent.hpp"
#include "2dfm/2dfmScriptItem.hpp"
#include "2dfm/CommonResource.hpp"
#include "MoveComponent.hpp"
#include "engine/AudioSystem.hpp"
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
    spriteComponent = dynamic_cast<Sprite *>(_owner->getChildByName("SpriteComponent"));
    moveComponent = dynamic_cast<MoveComponent *>(_owner->getComponent("MoveComponent"));

    if (!spriteComponent || !moveComponent) {
        throw std::runtime_error("Can't get required components!");
    }
}

void ScriptInterceptorComponent::setRunningScript(int scriptIdx) {
    auto &script = getCommonResource()->scripts.at(scriptIdx);
    startIdx = script.startIdx;
    endIdx = script.endIdx;
    runningScriptItemIdx = startIdx;
}

void ScriptInterceptorComponent::interceptPlaySoundCmd(const _2dfm::PlaySoundCmd *cmd) {
    auto soundClip = getCommonResource()->sounds.at(cmd->soundIdx);
    AudioSystem::getInstance()->playClip(soundClip, soundClip->isLoop(), 1.f);
}

bool ScriptInterceptorComponent::hasNoShowPicItem() const {
    for (int i = startIdx; i < endIdx; ++i) {
        auto item = getCommonResource()->scriptItems[i];
        if (static_cast<_2dfm::DemoScriptItemTypes>(item->type) == _2dfm::DemoScriptItemTypes::PIC) {
            return false;
        }
    }
    return true;
}

_2dfm::ShowPic *ScriptInterceptorComponent::interceptScriptUntilShowPic() {
    for (int i = runningScriptItemIdx + 1; i < endIdx; ++i) {
        auto item = getCommonResource()->scriptItems[i];
        auto type = static_cast<_2dfm::DemoScriptItemTypes>(item->type);

        if (type == _2dfm::DemoScriptItemTypes::PIC) {
            runningScriptItemIdx = i;
            return reinterpret_cast<_2dfm::ShowPic *>(item);
        }
        if (type == _2dfm::DemoScriptItemTypes::END) {
            spriteComponent->setTexture(nullptr);
            spriteComponent->setVisible(false);
            return nullptr;
        }

        if (type == _2dfm::DemoScriptItemTypes::SOUND) {
            interceptPlaySoundCmd(reinterpret_cast<_2dfm::PlaySoundCmd *>(item));
        } else if (type == _2dfm::DemoScriptItemTypes::COLOR) { // 色
            interceptColorSetCmd(reinterpret_cast<_2dfm::ColorSet *>(item));
        } else if (type == _2dfm::DemoScriptItemTypes::MOVE) {
            auto moveCmd = reinterpret_cast<_2dfm::MoveCmd *>(item);
            auto vel       = ax::Vec2(moveCmd->moveX, moveCmd->moveY) * 0.01f;
            auto accel     = ax::Vec2(moveCmd->accelX, moveCmd->accelY) * 0.01f;
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
    }
    // 如果走到了最后，发现从头到尾都没有图片指令，则退出播放
    if (hasNoShowPicItem()) {
        spriteComponent->setVisible(false);
        return nullptr;
    } else {
        // 只要没遇到“完”，就会从头开始
        runningScriptItemIdx = startIdx;
        return interceptScriptUntilShowPic();
    }
}

void ScriptInterceptorComponent::interceptShowPicCmd(const _2dfm::ShowPic *cmd) {
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
    } else {
        spriteComponent->setTexture(nullptr);
        spriteComponent->setVisible(false);
    }
}

void ScriptInterceptorComponent::interceptColorSetCmd(const _2dfm::ColorSet *cmd) {
    auto setType = static_cast<_2dfm::ColorSetType>(cmd->colorSetType);
    switch (setType) {
    case _2dfm::ColorSetType::ALPHA_BLEND: {
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
    case _2dfm::ColorSetType::TRANSPARENCY:
        spriteComponent->setVisible(true);
        spriteComponent->setBlendFunc(BlendFunc::ALPHA_NON_PREMULTIPLIED);
        spriteComponent->setOpacity(128);
        break;
    case _2dfm::ColorSetType::ADD_BLEND:
        spriteComponent->setVisible(true);
        spriteComponent->setBlendFunc(BlendFunc::ADDITIVE);
        spriteComponent->setOpacity(255);
        break;
    case _2dfm::ColorSetType::MINUS_BLEND:
        spriteComponent->setVisible(true);
        spriteComponent->setBlendFunc({backend::BlendFactor::ONE, backend::BlendFactor::ONE_MINUS_SRC_COLOR});
        spriteComponent->setOpacity(255);
        break;
    case _2dfm::ColorSetType::NORMAL:
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
