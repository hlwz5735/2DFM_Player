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
    ax::Vec2 getOffset() const { return offset; }
    void setPosition(float x, float y) override;

private:
    ax::Vec2 lastPosition;
    ax::Vec2 offset;
};

#endif //STAGECAMERANODE_HPP
