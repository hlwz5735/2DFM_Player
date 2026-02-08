//
// Created by limen on 2025/1/10.
//

#include "StageNode.hpp"

#include "MoveComponent.hpp"
#include "ParallaxComponent.hpp"
#include "SeamlessScrollComponent.hpp"

void StageNode::update(float delta) {
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
        const auto parallaxScale = parallaxComp->getParallaxScale();
        auto virtualHeight = visibleHeight / 2 * (1 + parallaxScale.y);
        setPosition(
            cameraOffset.x * parallaxScale.x,
            virtualHeight + cameraOffset.y * parallaxScale.y);
    } else {
        setPosition(logicPosition.x, visibleHeight - logicPosition.y);
    }
}

void StageNode::addParallaxComp(ParallaxComponent *comp) {
    AXASSERT(parallaxComp == nullptr, "ParallaxComponent should be set only once.");
    parallaxComp = comp;
    addComponent(parallaxComp);
}
void StageNode::addSeamlessComp(SeamlessScrollComponent *comp) {
    AXASSERT(seamlessScrollComp == nullptr, "SeamlessScrollComponent should be set only once.");
    seamlessScrollComp = comp;
    addComponent(seamlessScrollComp);
}
