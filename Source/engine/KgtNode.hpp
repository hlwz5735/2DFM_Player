//
// Created by limen on 2024/11/23.
//

#ifndef KGTNODE_HPP
#define KGTNODE_HPP

#include <axmol.h>

class KgtNode : public ax::Node {
public:
    bool initWithVisibleHeight(int visibleHeight = 640);
    bool init() override { return initWithVisibleHeight(static_cast<int>(_director->getVisibleSize().height)); }
    void update(float delta) override;

    const ax::Vec2 &getLogicPosition() const { return logicPosition; }
    void setLogicPosition(const ax::Vec2 &pos);

protected:
    ax::Vec2 logicPosition = ax::Vec2::ZERO;
    ax::Sprite *spriteComponent = nullptr;
    class MoveComponent *moveComp = nullptr;
    class ParallaxComponent *parallaxComponent = nullptr;
    class SeamlessScrollComponent *seamlessScrollComp = nullptr;
    class ScriptInterceptorComponent *interceptorComp = nullptr;
private:
    int visibleHeight = 0;
};

#endif //KGTNODE_HPP
