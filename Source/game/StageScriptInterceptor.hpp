#pragma once

#include "2dfm/KgtStage.hpp"
#include "ScriptInterceptorComponent.hpp"

class StageScriptInterceptor : public ScriptInterceptorComponent {
public:
    void update(float deltaTime) override;

    const CommonResource *getCommonResource() const override { return stageData; }
    void setStageData(KgtStage *stage) { stageData = stage; }
    void setVirtualCameraNode(ax::Node *cameraNode) { virtualCameraNode = cameraNode; }
private:
    KgtStage *stageData = nullptr;
    ax::Node *virtualCameraNode = nullptr;
};
