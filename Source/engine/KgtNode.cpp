//
// Created by limen on 2024/11/23.
//
#include "KgtNode.hpp"
#include <axmol.h>
#include "game/MoveComponent.hpp"
#include "game/ScriptInterceptorComponent.hpp"

USING_NS_AX;

bool KgtNode::initWithVisibleHeight(int visibleHeight) {
    if (!Node::init()) {
        return false;
    }
    this->visibleHeight = visibleHeight;

    moveComp = utils::createInstance<MoveComponent>();
    moveComp->setName("MoveComponent");
    this->addComponent(moveComp);

    spritePNode = utils::createInstance<Node>();
    this->addChild(spritePNode);

    spriteComp = utils::createInstance<Sprite>();
    spriteComp->setName("SpriteComponent");
    spriteComp->setPosition(Vec2::ZERO);
    spriteComp->setAnchorPoint(Vec2(0, 1));
    spritePNode->addChild(spriteComp);

    return true;
}

void KgtNode::update(float delta) {
    Node::update(delta);

    // 处理移动
    auto transOffset = moveComp->getVelocity() * delta * 100;
    transOffset.y = -transOffset.y;
    spritePNode->setPosition(spritePNode->getPosition() + transOffset);

    setPosition(logicPosition.x, visibleHeight - logicPosition.y);
}

void KgtNode::destroy() {
    if (auto interceptor = getInterceptor()) {
        interceptor->stop();
    }
    // 如果挂载到了父节点，从父节点中移除
    if (_parent) {
        _parent->removeChild(this);
    }
}

void KgtNode::setLogicPosition(const Vec2 &pos) {
    logicPosition = pos;
}

void KgtNode::addInterceptor(ScriptInterceptorComponent *p) {
    AXASSERT(this->interceptorComp == nullptr, "Interceptor already set");
    p->setName("ScriptInterceptorComponent");
    this->addComponent(p);
    interceptorComp = p;
}
