#include "DemoScriptInterceptor.hpp"

#include "../2dfm/2dfmScriptItem.hpp"
#include "../2dfm/KgtDemo.hpp"
#include "../base/SpriteComponent.hpp"
#include "../base/Game.hpp"
#include "../base/AudioSystem.hpp"

DemoScriptInterceptor::DemoScriptInterceptor(Node *owner, int updateOrder)
        : Component(owner, updateOrder) {
    spriteComponent = owner->getComponent<SpriteComponent>();
}

void DemoScriptInterceptor::update(float deltaTime) {
    Component::update(deltaTime);
    if (!spriteComponent || !demoData) {
        return;
    }
    if (timeWaiting == std::numeric_limits<float>::infinity()) {
        return;
    }
    if (hasNoShowPicItem()) {
        return;
    }
    if (timeWaiting > 0) {
        timeWaiting -= deltaTime;
    }
    const _2dfm::ShowPic *showPicScript = nullptr;
    while (timeWaiting <= 0) {
        showPicScript = getNextPicToShow();
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
        if (static_cast<int>(item->type) == _2dfm::DemoScriptItemTypes::PIC) {
            return false;
        }
    }
    return true;
}

_2dfm::ShowPic *DemoScriptInterceptor::getNextPicToShow() {
    for (int i = runningScriptItemIdx + 1; i < endIdx; ++i) {
        auto item = demoData->scriptItems[i];
        if (item->type == _2dfm::DemoScriptItemTypes::PIC) {
            runningScriptItemIdx = i;
            return reinterpret_cast<_2dfm::ShowPic *>(item);
        } else if (item->type == _2dfm::DemoScriptItemTypes::SOUND) {
            auto soundScript = reinterpret_cast<_2dfm::PlaySound *>(item);
            auto soundClip = demoData->sounds.at(soundScript->soundIdx);
            Game::getInstance()->getAudioSystem()->playClip(soundClip);
        }
    }
    return nullptr;
}
