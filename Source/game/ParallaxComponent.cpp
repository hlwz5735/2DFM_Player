//
// Created by limen on 2024/11/25.
//

#include "ParallaxComponent.hpp"
#include "GameConfig.hpp"
#include "StageCameraNode.hpp"

USING_NS_AX;

bool ParallaxComponent::init(StageCameraNode *cameraNode) {
    if (!Component::init()) {
        return false;
    }
    this->setName("ParallaxComponent");
    this->virtualCamera = cameraNode;
    return true;
}

Vec2 ParallaxComponent::getOffset() const {
    if (!virtualCamera) {
        return Vec2::ZERO;
    }
    const auto &cameraPos = virtualCamera->getPosition();
    return -cameraPos;
}
