#pragma once

#include "2dfm/KgtStage.hpp"
#include "ScriptInterceptorComponent.hpp"

class StageScriptInterceptor : public ScriptInterceptorComponent {
public:
    void update(float deltaTime) override;

    const CommonResource *getCommonResource() const override { return stageData; }

    void setStageData(KgtStage *stage);
protected:
    void interceptShowPicCmd(const _2dfm::ShowPic *cmd) override;
private:
    KgtStage *stageData = nullptr;
};
