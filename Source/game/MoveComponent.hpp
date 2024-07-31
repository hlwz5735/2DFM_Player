#pragma once

#include <2d/Component.h>
#include <math/Vec2.h>

class MoveComponent : public ax::Component {
public:
    void update(float deltaTime) override;

    const ax::Vec2 &getVelocity() const { return velocity; }
    const ax::Vec2 &getAcceleration() const { return acceleration; }
    void setVelocity(const ax::Vec2 &velocity) { this->velocity = velocity; }
    void setAcceleration(const ax::Vec2 &acceleration) { this->acceleration = acceleration; }

private:
    ax::Vec2 velocity = ax::Vec2::ZERO;
    ax::Vec2 acceleration = ax::Vec2::ZERO;
};
