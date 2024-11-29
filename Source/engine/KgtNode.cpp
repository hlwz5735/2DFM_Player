//
// Created by limen on 2024/11/23.
//
#include "KgtNode.hpp"
#include <axmol.h>
#include "game/MoveComponent.hpp"
#include "game/ParallaxComponent.hpp"
#include "game/SeamlessScrollComponent.hpp"

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

    // 处理无缝滚动
    if (seamlessScrollComp) {
        seamlessScrollComp->lateUpdate(delta);
    }

    // 处理视口变换
    if (parallaxComp) {
        auto cameraOffset = parallaxComp->getOffset();
        auto &parallaxScale = parallaxComp->getParallaxScale();
        auto virtualHeight = visibleHeight / 2 * (1 + parallaxScale.y);
        setPosition(
            cameraOffset.x * parallaxScale.x,
            virtualHeight + cameraOffset.y * parallaxScale.y);
    } else {
        setPosition(logicPosition.x, visibleHeight - logicPosition.y);
    }
}

void KgtNode::setLogicPosition(const Vec2 &pos) {
    logicPosition = pos;
}
void KgtNode::addParallaxComp(class ParallaxComponent *comp) {
    ASSERT(parallaxComp == nullptr, "ParallaxComponent should be set only once.");
    parallaxComp = comp;
    addComponent(parallaxComp);
}
void KgtNode::addSeamlessComp(class SeamlessScrollComponent *comp) {
    ASSERT(seamlessScrollComp == nullptr, "SeamlessScrollComponent should be set only once.");
    seamlessScrollComp = comp;
    addComponent(seamlessScrollComp);
}
