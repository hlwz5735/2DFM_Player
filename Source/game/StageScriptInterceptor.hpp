#pragma once

#include "2dfm/KgtStage.hpp"
#include "ScriptInterceptorComponent.hpp"
#include <memory>

class StageScriptInterceptor : public ScriptInterceptorComponent {
public:
    void update(float deltaTime) override;

    const CommonResource *getCommonResource() const override { return stageData.get(); }

    void setStageData(std::shared_ptr<KgtStage> stage);
protected:
    void interceptShowPicCmd(const _2dfm::ShowPic *cmd) override;
private:
    std::shared_ptr<KgtStage> stageData;
};
