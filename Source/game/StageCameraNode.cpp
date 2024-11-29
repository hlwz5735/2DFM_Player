//
// Created by limen on 2024/11/25.
//

#include "StageCameraNode.hpp"

#include "GameConfig.hpp"
#include "engine/Input.hpp"

USING_NS_AX;

bool StageCameraNode::init() {
    if (!Node::init()) {
        return false;
    }

    this->scheduleUpdate();

    return true;
}

void StageCameraNode::update(float delta) {
    Node::update(delta);
    Vec2 offset;
    auto dpad = Input::getInstance().getDPad();
    if (dpad == Input::DPadDir::LEFT_DOWN) {
        offset = Vec2(-cameraSpeed, -cameraSpeed);
    } else if (dpad == Input::DPadDir::DOWN) {
        offset = Vec2(0, -cameraSpeed);
    } else if (dpad == Input::DPadDir::RIGHT_DOWN) {
        offset = Vec2(cameraSpeed, -cameraSpeed);
    } else if (dpad == Input::DPadDir::LEFT) {
        offset = Vec2(-cameraSpeed, 0);
    } else if (dpad == Input::DPadDir::RIGHT) {
        offset = Vec2(cameraSpeed, 0);
    } else if (dpad == Input::DPadDir::LEFT_UP) {
        offset = Vec2(-cameraSpeed, cameraSpeed);
    } else if (dpad == Input::DPadDir::UP) {
        offset = Vec2(0, cameraSpeed);
    } else if (dpad == Input::DPadDir::RIGHT_UP) {
        offset = Vec2(cameraSpeed, cameraSpeed);
    } else {
        offset = Vec2::ZERO;
    }

    lastPosition = getPosition();
    if (offset != Vec2::ZERO) {
        this->setPosition(lastPosition + offset);
    }
}

void StageCameraNode::setPosition(float x, float y) {
    Node::setPosition(
        clampf(x, 0, GameConfig::stageWidth / 2),
        clampf(y, 0, GameConfig::stageHeight / 2)
    );
}

void StageCameraNode::reset() {
    lastPosition = Vec2::ZERO;
}
