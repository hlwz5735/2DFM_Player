#pragma once

#include "../base/Component.hpp"

struct KgtDemo;

namespace _2dfm {
    struct ShowPic;
}

class DemoScriptInterceptor : public Component {
public:
    DemoScriptInterceptor(Node *owner, int updateOrder = 10);

    void update(float deltaTime) override;

    void setDemoData(KgtDemo *data) { demoData = data; }
    void setRunningScript(int scriptIdx);
private:
    bool hasNoShowPicItem() const;
    _2dfm::ShowPic *interceptScriptUntilShowPic();

    class SpriteComponent *spriteComponent = nullptr;
    class MoveComponent *moveComponent = nullptr;
    KgtDemo *demoData = nullptr;

    int runningScriptItemIdx = 0;
    int startIdx = 0;
    int endIdx = 0;

    float timeWaiting = 0;
};
