//
// Created by limen on 2024/11/25.
//

#ifndef STAGECAMERANODE_HPP
#define STAGECAMERANODE_HPP

#include <axmol.h>

class StageCameraNode : public ax::Node {
public:
    bool init() override;
    void update(float delta) override;

    ax::Vec2 getOffset() const { return _position - lastPosition; }
    ax::Size getViewSize() const { return _director->getVisibleSize(); }

    void setPosition(float x, float y) override;
    void setPosition(const ax::Vec2 &pos) override { StageCameraNode::setPosition(pos.x, pos.y); }

    float getCameraSpeed() const { return cameraSpeed; }
    void setCameraSpeed(float speed) { cameraSpeed = speed; }

    void reset();

private:
    ax::Vec2 lastPosition;
    float cameraSpeed = 5;
};

#endif //STAGECAMERANODE_HPP
