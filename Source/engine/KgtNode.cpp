//
// Created by limen on 2024/11/23.
//
#include "KgtNode.hpp"
#include <axmol.h>
#include "game/MoveComponent.hpp"

USING_NS_AX;

bool KgtNode::initWithVisibleHeight(int visibleHeight) {
    if (!Node::init()) {
        return false;
    }
    this->visibleHeight = visibleHeight;

    auto mc = utils::createInstance<MoveComponent>();
    mc->setName("MoveComponent");
    this->addComponent(mc);

    auto sprite = utils::createInstance<Sprite>();
    sprite->setName("SpriteComponent");
    sprite->setPosition(Vec2::ZERO);
    sprite->setAnchorPoint(Vec2(0, 1));
    this->addChild(sprite);

    return true;
}

void KgtNode::update(float delta) {
    Node::update(delta);
    setPosition(logicPosition.x, visibleHeight - logicPosition.y);
}

void KgtNode::setLogicPosition(const Vec2 &pos) {
    logicPosition = pos;
}
