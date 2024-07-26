#include "MoveComponent.hpp"

#include "Node.hpp"

void MoveComponent::update(float deltaTime) {
    const float delta = deltaTime * 100;

    Component::update(deltaTime);
    const auto oriPos = owner->getPosition();
    this->velocity += acceleration * delta;
    owner->setPosition(oriPos + Vector2(velocity.x, -velocity.y) * delta);
}
