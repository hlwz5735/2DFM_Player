#pragma once

#include "ScriptInterceptorComponent.hpp"
#include "2dfm/KgtDemo.hpp"
#include <memory>

class DemoScriptInterceptor : public ScriptInterceptorComponent {
public:
    void update(float deltaTime) override;

    const CommonResource *getCommonResource() const override { return demoData.get(); }

    void setDemoData(std::shared_ptr<KgtDemo> data);
private:
    std::shared_ptr<KgtDemo> demoData;
    /// 播放时间（到达上限后自动结束）
    float playTimer = 0;
};
