#include "DemoScriptInterceptor.hpp"
#include "../2dfm/2dfmScriptItem.hpp"
#include "../2dfm/KgtDemo.hpp"
#include "../base/MoveComponent.hpp"
#include <2d/Node.h>

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

bool DemoScriptInterceptor::init() {
    if (!Component::init()) {
        return false;
    }
    return true;
}

void DemoScriptInterceptor::onAdd() {
    Component::onAdd();
    spriteComponent = dynamic_cast<Sprite *>(_owner->getChildByName("SpriteComponent"));
    moveComponent = dynamic_cast<MoveComponent *>(_owner->getComponent("MoveComponent"));

    if (!spriteComponent || !moveComponent) {
        throw std::runtime_error("Can't get required components!");
    }
}

void DemoScriptInterceptor::update(float deltaTime) {
    Component::update(deltaTime);
    if (!spriteComponent || !demoData) {
        return;
    }
    if (timeWaiting == std::numeric_limits<float>::infinity()) {
        return;
    }
    if (runningScriptItemIdx == endIdx) {
        return;
    }
    if (timeWaiting > 0) {
        timeWaiting -= deltaTime;
    }
    playTimer += deltaTime;
    if (demoData->config.totalTime != 0 && playTimer * 100 >= demoData->config.totalTime) {
        // TODO: 结束播放并向后跳转
    }
    const _2dfm::ShowPic *showPicScript = nullptr;
    while (timeWaiting <= 0) {
        showPicScript = interceptScriptUntilShowPic();
        if (!showPicScript) {
            timeWaiting = std::numeric_limits<float>::infinity();
            runningScriptItemIdx = endIdx;
            return;
        }
        float keepTime = static_cast<float>(showPicScript->keepTime) / 100;
        if (keepTime == 0) {
            timeWaiting = std::numeric_limits<float>::infinity();
        } else {
            timeWaiting += keepTime;
        }
    }
    if (showPicScript) {
        auto tex = demoData->pictures.at(showPicScript->getPicIdx());
        if (tex) {
            const auto blendFunc = spriteComponent->getBlendFunc();
            spriteComponent->setTexture(tex);
            const auto &r = tex->getContentSizeInPixels();
            spriteComponent->setTextureRect({0, 0, r.width, r.height});
            spriteComponent->setPosition(showPicScript->getOffset().x, -showPicScript->getOffset().y);
            spriteComponent->setVisible(true);
            spriteComponent->setBlendFunc(blendFunc);
        } else {
            spriteComponent->setTexture(nullptr);
            spriteComponent->setVisible(false);
        }
    }
}

void DemoScriptInterceptor::setRunningScript(int scriptIdx) {
    auto &script = demoData->scripts.at(scriptIdx);
    startIdx = script.startIdx;
    endIdx = script.endIdx;
    runningScriptItemIdx = startIdx;
}

bool DemoScriptInterceptor::hasNoShowPicItem() const {
    for (int i = startIdx; i < endIdx; ++i) {
        auto item = demoData->scriptItems[i];
        if (static_cast<_2dfm::DemoScriptItemTypes>(item->type) == _2dfm::DemoScriptItemTypes::PIC) {
            return false;
        }
    }
    return true;
}

_2dfm::ShowPic *DemoScriptInterceptor::interceptScriptUntilShowPic() {
    for (int i = runningScriptItemIdx + 1; i < endIdx; ++i) {
        auto item = demoData->scriptItems[i];
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
            auto soundScript = reinterpret_cast<_2dfm::PlaySoundCmd *>(item);
            auto soundClip = demoData->sounds.at(soundScript->soundIdx);
            //Game::getInstance()->getAudioSystem()->playClip(soundClip);
            //ax::AudioEngine::play2d();
        } else if (type == _2dfm::DemoScriptItemTypes::COLOR) { // 色
            auto colorSet = reinterpret_cast<_2dfm::ColorSet *>(item);
            auto setType = static_cast<_2dfm::ColorSetType>(colorSet->colorSetType);
            switch (setType) {
            case _2dfm::ColorSetType::ALPHA_BLEND:
                spriteComponent->setBlendFunc(BlendFunc::ALPHA_NON_PREMULTIPLIED);
                spriteComponent->setOpacity(convertKgtOpacityColorValue(colorSet->alpha));
                break;
            case _2dfm::ColorSetType::TRANSPARENCY:
                spriteComponent->setBlendFunc(BlendFunc::ALPHA_NON_PREMULTIPLIED);
                spriteComponent->setOpacity(128);
                break;
            case _2dfm::ColorSetType::ADD_BLEND:
                spriteComponent->setBlendFunc(BlendFunc::ADDITIVE);
                spriteComponent->setOpacity(255);
                break;
            case _2dfm::ColorSetType::MINUS_BLEND:
                spriteComponent->setBlendFunc({backend::BlendFactor::ONE, backend::BlendFactor::ONE_MINUS_SRC_COLOR});
                spriteComponent->setOpacity(255);
                break;
            case _2dfm::ColorSetType::NORMAL:
            default:
                spriteComponent->setBlendFunc(BlendFunc::ALPHA_NON_PREMULTIPLIED);
                spriteComponent->setOpacity(255);
                break;
            }
            // TODO: 颜色叠加暂时关闭
            /*const ax::Color3B blendColor(convertKgtRgbColorValue(colorSet->red),
                convertKgtRgbColorValue(colorSet->green),
                convertKgtRgbColorValue(colorSet->blue));
            spriteComponent->setColor(blendColor);*/
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
