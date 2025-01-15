//
// Created by limen on 2024/11/23.
//

#ifndef KGTNODE_HPP
#define KGTNODE_HPP

#include <axmol.h>

class ScriptInterceptorComponent;

class KgtNode : public ax::Node {
public:
    friend class SeamlessScrollComponent;

    virtual bool initWithVisibleHeight(int visibleHeight = 640);
    bool init() override { return initWithVisibleHeight(static_cast<int>(_director->getVisibleSize().height)); }
    void update(float delta) override;
    void destroy();

    const ax::Vec2 &getLogicPosition() const { return logicPosition; }
    void setLogicPosition(const ax::Vec2 &pos);

    ax::Sprite *getSprite() const { return spriteComp; }
    class MoveComponent *getMoveComp() const { return moveComp; }

    ScriptInterceptorComponent *getInterceptor() const { return interceptorComp; }
    void addInterceptor(ScriptInterceptorComponent *p);

protected:
    ax::Vec2 logicPosition = ax::Vec2::ZERO;
    ax::Sprite *spriteComp = nullptr;
    ax::Node *spritePNode = nullptr;
    class MoveComponent *moveComp = nullptr;
    class ScriptInterceptorComponent *interceptorComp = nullptr;

    int visibleHeight = 0;
};

#endif //KGTNODE_HPP
