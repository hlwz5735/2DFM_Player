//
// Created by limen on 2024/11/23.
//

#include "KgtScriptInterceptor.hpp"
#include "2dfm/2dfmScriptItem.hpp"

void KgtScriptInterceptor::update(float deltaTime) {
    ScriptInterceptorComponent::update(deltaTime);
    if (!spriteComponent || !kgtGame) {
        return;
    }
    if (timeWaiting == std::numeric_limits<float>::infinity()) {
        return;
    }
    if (runningStack.empty()) {
        return;
    }
    if (runningScriptItemIdx == runningStack.back().scriptItemEndIdx) {
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
            runningScriptItemIdx = runningStack.back().scriptItemEndIdx;
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
        this->interceptShowPicCmd(showPicScript);
    }
}
