#include "DemoScriptInterceptor.hpp"

#include "../2dfm/2dfmScriptItem.hpp"
#include "../2dfm/KgtDemo.hpp"
#include "../base/SpriteComponent.hpp"

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
    timeWaiting -= deltaTime;
    const _2dfm::ShowPic *showPicScript = nullptr;
    while (timeWaiting <= 0) {
        ++runningScriptItemIdx;
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
            timeWaiting += static_cast<float>(showPicScript->keepTime) / 100;
        }
    }
    auto tex = demoData->pictures.at(showPicScript->getPicIdx());
    spriteComponent->setTexture(tex);
    spriteComponent->setOffset(showPicScript->getOffset());
}

void DemoScriptInterceptor::setRunningScript(int scriptIdx) {
    auto script = demoData->scripts.at(scriptIdx);
    startIdx = script->scriptIndex;
    if (scriptIdx == demoData->scripts.size() - 1) {
        endIdx = demoData->scriptItems.size();
    } else {
        auto nextScript = demoData->scripts.at(scriptIdx + 1);
        endIdx = nextScript->scriptIndex;
    }
    runningScriptItemIdx = startIdx;
}

bool DemoScriptInterceptor::hasNoShowPicItem() const {
    for (int i = startIdx; i < endIdx; ++i) {
        auto item = demoData->scriptItems[i];
        if (static_cast<int>(item->type) == _2dfm::DemoScriptItemTypes::PIC) {
            return false;
        }
    }
    return true;
}

_2dfm::ShowPic *DemoScriptInterceptor::getNextPicToShow() const {
    for (int i = runningScriptItemIdx; i < endIdx; ++i) {
        auto item = demoData->scriptItems[i];
        if (static_cast<int>(item->type) == _2dfm::DemoScriptItemTypes::PIC) {
            return reinterpret_cast<_2dfm::ShowPic *>(item);
        }
    }
    return nullptr;
}
