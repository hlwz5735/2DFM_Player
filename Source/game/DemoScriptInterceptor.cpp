#include "DemoScriptInterceptor.hpp"
#include "2dfm/2dfmScriptItem.hpp"

USING_NS_AX;

void DemoScriptInterceptor::update(float deltaTime) {
    ScriptInterceptorComponent::update(deltaTime);
    if (!spriteComponent || !demoData) {
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
    playTimer += deltaTime;
    if (demoData->config.totalTime != 0 && playTimer * 100 >= demoData->config.totalTime) {
        // TODO: 结束播放并向后跳转
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

void DemoScriptInterceptor::setDemoData(KgtDemo *data) {
    this->stop();
    this->demoData = data;
}
