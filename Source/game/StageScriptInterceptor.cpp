#include "StageScriptInterceptor.hpp"
#include "2dfm/2dfmScriptItem.hpp"
#include "2dfm/KgtStage.hpp"
#include "MoveComponent.hpp"
#include "SeamlessScrollComponent.hpp"
#include "StageNode.hpp"

USING_NS_AX;

// 脚本推进（含跳帧补偿）统一在 ScriptInterceptorComponent::update 里做

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
