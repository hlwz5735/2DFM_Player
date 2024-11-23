#pragma once

#include "ScriptInterceptorComponent.hpp"
#include "2dfm/KgtDemo.hpp"

class DemoScriptInterceptor : public ScriptInterceptorComponent {
public:
    void update(float deltaTime) override;
    void setDemoData(KgtDemo *data) { demoData = data; }

    const CommonResource *getCommonResource() const override { return demoData; }

private:
    KgtDemo *demoData = nullptr;
    /// 播放时间（到达上限后自动结束）
    float playTimer = 0;
};
