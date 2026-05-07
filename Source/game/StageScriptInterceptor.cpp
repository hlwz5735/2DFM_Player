#include "StageScriptInterceptor.hpp"
#include "2dfm/2dfmScriptItem.hpp"
#include "2dfm/KgtStage.hpp"
#include "MoveComponent.hpp"
#include "SeamlessScrollComponent.hpp"
#include "StageNode.hpp"

USING_NS_AX;

void StageScriptInterceptor::update(float deltaTime) {
    ScriptInterceptorComponent::update(deltaTime);
    if (!spriteComponent || !stageData) {
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

void StageScriptInterceptor::setStageData(std::shared_ptr<KgtStage> stage) {
    this->stop();
    this->stageData = std::move(stage);
}

void StageScriptInterceptor::interceptShowPicCmd(const _2dfm::ShowPic *cmd) {
    auto oldTex = spriteComponent->getTexture();
    const auto tex = getCommonResource()->pictures.at(cmd->getPicIdx());

    ScriptInterceptorComponent::interceptShowPicCmd(cmd);

    if (auto owner = dynamic_cast<StageNode *>(_owner)) {
        auto seamlessComp = owner->getSeamlessScrollComp();
        if (seamlessComp && oldTex != tex) {
            seamlessComp->updateSprite();
        }
    }
}
