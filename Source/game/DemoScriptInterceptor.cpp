#include "DemoScriptInterceptor.hpp"
#include "2dfm/2dfmScriptItem.hpp"

USING_NS_AX;

// 脚本推进（含跳帧补偿）统一在 ScriptInterceptorComponent::update 里做，
// 这里只补上 DEMO 自己的播放计时
void DemoScriptInterceptor::onAdvanceTick(float deltaTime) {
    ScriptInterceptorComponent::onAdvanceTick(deltaTime);
    playTimer += deltaTime;
    if (demoData->config.totalTime != 0 && playTimer * 100 >= demoData->config.totalTime) {
        // TODO: 结束播放并向后跳转
    }
}

void DemoScriptInterceptor::setDemoData(std::shared_ptr<KgtDemo> data) {
    this->stop();
    this->demoData = std::move(data);
}
