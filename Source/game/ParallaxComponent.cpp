//
// Created by limen on 2024/11/25.
//

#include "ParallaxComponent.hpp"
#include "StageCameraNode.hpp"

bool ParallaxComponent::init(StageCameraNode *cameraNode) {
    if (!Component::init()) {
        return false;
    }
    this->virtualCamera = cameraNode;
    return true;
}
void ParallaxComponent::onAdd() {
    Component::onAdd();
    originPos = _owner->getPosition();
}
void ParallaxComponent::update(float delta) {
    Component::update(delta);
    if (!virtualCamera) {
        return;
    }
    auto cLoc = virtualCamera->getOffset();
    _owner->setPosition(
        _owner->getPosition().x - cLoc.x * parallaxScaleX,
        _owner->getPosition().y - cLoc.y * parallaxScaleY);
}
