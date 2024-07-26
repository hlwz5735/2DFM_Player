//
// Created by 厉猛 on 2024-07-16.
//
#include "Node.hpp"
#include "Game.hpp"
#include "Component.hpp"

Node::Node(Game *game): state(State::ACTIVE), position(Vector2::ZERO), game(game) {
    game->addGameObject(this);
}

Node::~Node() {
    game->removeGameObject(this);

    while(!components.empty()) {
        delete components.back();
    }
}

void Node::addComponent(Component *component) {
    int order = component->getUpdateOrder();
    auto iter = components.begin();
    for(; iter != components.end(); ++iter) {
        if (order < (*iter)->getUpdateOrder()) {
            break;
        }
    }
    components.insert(iter, component);
}

void Node::removeComponent(Component *component) {
    auto iter = std::find(components.begin(), components.end(), component);
    if (iter != components.end()) {
        components.erase(iter);
    }
}

void Node::__update(float deltaTime) {
    if (state == State::ACTIVE) {
        updateComponents(deltaTime);
        update(deltaTime);
    }
}

void Node::updateComponents(float deltaTime) {
    for (auto c : components) {
        c->update(deltaTime);
    }
}
