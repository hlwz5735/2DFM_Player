#pragma once

#include "Component.hpp"
#include "../math/Vector.hpp"

class MoveComponent : public Component {
public:
    MoveComponent(Node *owner, int updateOrder =10): Component(owner, updateOrder)
        , velocity(Vector2::ZERO), acceleration(Vector2::ZERO) {}

    void update(float deltaTime) override;

    const Vector2 & getVelocity() const { return velocity; }
    const Vector2 & getAcceleration() const { return acceleration; }
    void setVelocity(const Vector2 &velocity) { this->velocity = velocity; }
    void setAcceleration(const Vector2 &acceleration) { this->acceleration = acceleration; }

private:
    Vector2 velocity;
    Vector2 acceleration;
};
