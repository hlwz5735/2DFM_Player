#include "MoveComponent.hpp"
#include <2d/Node.h>

USING_NS_AX;

void MoveComponent::update(float deltaTime) {
    const float delta = deltaTime * 100;

    Component::update(deltaTime);
    const auto oriPos = _owner->getPosition();
    this->velocity += acceleration * delta;
    _owner->setPosition(oriPos + ax::Vec2(velocity.x, -velocity.y) * delta);
}
