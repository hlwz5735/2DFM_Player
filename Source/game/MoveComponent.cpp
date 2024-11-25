#include "MoveComponent.hpp"
#include <2d/Node.h>

#include "engine/KgtNode.hpp"

USING_NS_AX;

void MoveComponent::update(float deltaTime) {
    Component::update(deltaTime);

    auto owner = dynamic_cast<KgtNode*>(_owner);

    const float delta = deltaTime * 100;
    const auto oriPos = owner->getLogicPosition();
    this->velocity += acceleration * delta;
    owner->setLogicPosition(oriPos + Vec2(velocity.x, velocity.y) * delta);
}
