//
// Created by limen on 2025/1/10.
//

#include "PlayerNode.hpp"

#include "MoveComponent.hpp"

USING_NS_AX;

bool PlayerNode::initWithVisibleHeight(int visibleHeight) {
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
    spriteComp->setAnchorPoint(Vec2(0.5f, 0));
    spritePNode->addChild(spriteComp);

    return true;
}
