#pragma once

#include "ScriptInterceptorComponent.hpp"
#include "2dfm/KgtDemo.hpp"
#include <memory>

class DemoScriptInterceptor : public ScriptInterceptorComponent {
public:
    const CommonResource *getCommonResource() const override { return demoData.get(); }

    void setDemoData(std::shared_ptr<KgtDemo> data);
protected:
    /// 每帧脚本推进之前：累计播放时间并判断是否到达总时长
    void onAdvanceTick(float deltaTime) override;
private:
    std::shared_ptr<KgtDemo> demoData;
    /// 播放时间（到达上限后自动结束）
    float playTimer = 0;
};
