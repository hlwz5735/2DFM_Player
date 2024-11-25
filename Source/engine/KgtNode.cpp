//
// Created by limen on 2024/11/23.
//
#include "KgtNode.hpp"
#include "game/MoveComponent.hpp"
#include <axmol.h>

USING_NS_AX;

bool KgtNode::init() {
    if (!Node::init()) {
        return false;
    }
    const auto visibleSize = _director->getVisibleSize();
    this->visibleHeight = visibleSize.height;

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

void KgtNode::setLogicPosition(const ax::Vec2 &pos) {
    setPosition(pos.x, visibleHeight - pos.y);
}
