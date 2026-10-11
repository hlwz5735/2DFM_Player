//
// Created by limen on 2024/11/25.
//

#include "StageCameraNode.hpp"

#include <algorithm>

#include "GameConfig.hpp"
#include "engine/KgtNode.hpp"

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

    lastPosition = getPosition();

    if (followTarget != nullptr) {
        setPosition(calcFollowPosition());
    }
}

Vec2 StageCameraNode::calcFollowPosition() const {
    const auto viewSize = _director->getVisibleSize();
    const auto &focus = followTarget->getLogicPosition();

    // 让目标落在视口中心：摄像机偏移 = 目标逻辑坐标 - 半个视口
    const float x = focus.x - viewSize.width * 0.5f;
    // 垂直方向默认不动（见 setFollowVertical）
    float y = getPositionY();
    if (followVertical) {
        // 逻辑坐标 Y 轴向下为正，而摄像机偏移是从场景顶部往下算的，两者方向相反
        y = (static_cast<float>(GameConfig::stageHeight) - focus.y) - viewSize.height * 0.5f;
    }
    return Vec2(x, y);
}

void StageCameraNode::setPosition(float x, float y) {
    const auto viewSize = _director->getVisibleSize();
    Node::setPosition(
        clampf(x, 0.f, std::max(0.f, static_cast<float>(GameConfig::stageWidth) - viewSize.width)),
        clampf(y, 0.f, std::max(0.f, static_cast<float>(GameConfig::stageHeight) - viewSize.height))
    );
}

void StageCameraNode::reset() {
    lastPosition = Vec2::ZERO;
    setPosition(0.f, 0.f);
}
