#pragma once

#include "ScriptInterceptorComponent.hpp"
#include "2dfm/KgtDemo.hpp"

class DemoScriptInterceptor : public ScriptInterceptorComponent {
public:
    void update(float deltaTime) override;

    const CommonResource *getCommonResource() const override { return demoData; }

    void setDemoData(KgtDemo *data);
private:
    KgtDemo *demoData = nullptr;
    /// 播放时间（到达上限后自动结束）
    float playTimer = 0;
};
