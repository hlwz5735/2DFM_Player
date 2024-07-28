#include "DemoScriptInterceptor.hpp"

#include "../2dfm/2dfmScriptItem.hpp"
#include "../2dfm/KgtDemo.hpp"
#include "../base/SpriteComponent.hpp"
#include "../base/MoveComponent.hpp"
#include "../base/Game.hpp"
#include "../base/AudioSystem.hpp"

namespace {
    float convertKgtRgbColorValue(byte rawByte) {
        return static_cast<float>(static_cast<int>(*(reinterpret_cast<char *>(&rawByte)))) / 32;
    }
}

DemoScriptInterceptor::DemoScriptInterceptor(Node *owner, int updateOrder)
        : Component(owner, updateOrder) {
    spriteComponent = owner->getComponent<SpriteComponent>();
    moveComponent = owner->getComponent<MoveComponent>();
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
        spriteComponent->setTexture(tex);
        spriteComponent->setOffset(showPicScript->getOffset());
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
            return nullptr;
        }

        if (type == _2dfm::DemoScriptItemTypes::SOUND) {
            auto soundScript = reinterpret_cast<_2dfm::PlaySound *>(item);
            auto soundClip = demoData->sounds.at(soundScript->soundIdx);
            Game::getInstance()->getAudioSystem()->playClip(soundClip);
        } else if (type == _2dfm::DemoScriptItemTypes::COLOR) { // 色
            auto colorSet = reinterpret_cast<_2dfm::ColorSet *>(item);
            auto setType = static_cast<_2dfm::ColorSetType>(colorSet->colorSetType);
            switch (setType) {
            case _2dfm::ColorSetType::ALPHA_BLEND:
                spriteComponent->setBlendType(BlendType::NORMAL);
                spriteComponent->setOpacity((32.f - static_cast<float>(colorSet->alpha)) / 32.f);
                break;
            case _2dfm::ColorSetType::TRANSPARENCY:
                spriteComponent->setBlendType(BlendType::NORMAL);
                spriteComponent->setOpacity(.5f);
                break;
            case _2dfm::ColorSetType::ADD_BLEND:
                spriteComponent->setBlendType(BlendType::ADD);
                spriteComponent->setOpacity(1.f);
                break;
            case _2dfm::ColorSetType::MINUS_BLEND:
                spriteComponent->setBlendType(BlendType::MINUS);
                spriteComponent->setOpacity(1.f);
                break;
            case _2dfm::ColorSetType::NORMAL:
            default:
                spriteComponent->setBlendType(BlendType::NORMAL);
                spriteComponent->setOpacity(1.f);
                break;
            }
            spriteComponent->setBlendColor(
                    Vector3(convertKgtRgbColorValue(colorSet->red),
                            convertKgtRgbColorValue(colorSet->green),
                            convertKgtRgbColorValue(colorSet->blue))
            );
        } else if (type == _2dfm::DemoScriptItemTypes::MOVE) {
            auto moveCmd = reinterpret_cast<_2dfm::MoveCmd *>(item);
            auto vel = Vector2(moveCmd->moveX, moveCmd->moveY) * 0.01f;
            auto accel = Vector2(moveCmd->accelX, moveCmd->accelY) * 0.01f;
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
        spriteComponent->setTexture(nullptr);
        return nullptr;
    } else {
        // 只要没遇到“完”，就会从头开始
        runningScriptItemIdx = startIdx;
        return interceptScriptUntilShowPic();
    }
}
