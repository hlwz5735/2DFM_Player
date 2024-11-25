//
// Created by limen on 2024/11/25.
//

#include "StageCameraNode.hpp"

#include "GameConfig.hpp"

bool StageCameraNode::init() {
    if (!Node::init()) {
        return false;
    }

    this->scheduleUpdate();

    return true;
}

void StageCameraNode::update(float delta) {
    Node::update(delta);
    offset = getPosition() - lastPosition;
    lastPosition = getPosition();
}

void StageCameraNode::setPosition(float x, float y) {
    Node::setPosition(
        ax::clampf(x, 0, GameConfig::getInstance().getStageSize().width / 2),
        ax::clampf(y, -GameConfig::getInstance().getStageSize().height / 2, 0));
}
