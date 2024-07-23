//
// Created by 厉猛 on 2024-07-16.
//

#include "Node.hpp"
#include "Component.hpp"

Component::Component(Node *owner, int updateOrder): owner(owner), updateOrder(updateOrder) {
    owner->addComponent(this);
}

Component::~Component() {
    owner->removeComponent(this);
}

void Component::update(float deltaTime) {}
