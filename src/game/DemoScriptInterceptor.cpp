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
    for (int i = runningScriptItemIdx; i < endIdx; ++i) {
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
            moveComponent->setVelocity(vel);
        }
    }
    spriteComponent->setTexture(nullptr);
    return nullptr;
}
