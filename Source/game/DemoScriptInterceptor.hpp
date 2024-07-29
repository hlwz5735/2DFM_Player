#pragma once

#include <2d/Component.h>
#include <2d/Sprite.h>

struct KgtDemo;

namespace _2dfm {
    struct ShowPic;
}

class DemoScriptInterceptor : public ax::Component {
public:
    bool init() override;
    void onAdd() override;
    void update(float deltaTime) override;

    void setDemoData(KgtDemo *data) { demoData = data; }
    void setRunningScript(int scriptIdx);
private:
    bool hasNoShowPicItem() const;
    _2dfm::ShowPic *interceptScriptUntilShowPic();

    ax::Sprite *spriteComponent = nullptr;
    class MoveComponent *moveComponent = nullptr;
    KgtDemo *demoData = nullptr;

    int runningScriptItemIdx = 0;
    int startIdx = 0;
    int endIdx = 0;

    float timeWaiting = 0;
    /// 播放时间（到达上限后自动结束）
    float playTimer = 0;
};
