#include "StageScriptInterceptor.hpp"
#include "2dfm/2dfmScriptItem.hpp"
#include "2dfm/KgtStage.hpp"
#include "MoveComponent.hpp"

USING_NS_AX;

void StageScriptInterceptor::update(float deltaTime) {
    ScriptInterceptorComponent::update(deltaTime);
    if (!spriteComponent || !stageData) {
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
        this->interceptShowPicCmd(showPicScript);
    }
}
