#include "MoveComponent.hpp"
#include <2d/Node.h>

#include "engine/KgtNode.hpp"

USING_NS_AX;

void MoveComponent::update(float deltaTime) {
    Component::update(deltaTime);
    const float delta = deltaTime * 100;
    this->velocity += acceleration * delta;
}
