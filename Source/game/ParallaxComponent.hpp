//
// Created by limen on 2024/11/25.
//

#ifndef PARALLAXCOMPONENT_HPP
#define PARALLAXCOMPONENT_HPP

#include <axmol.h>

#include "engine/KgtComponent.hpp"

class ParallaxComponent : public KgtComponent {
public:
    bool init(class StageCameraNode *cameraNode);

    const ax::Vec2 getParallaxScale() const { return ax::Vec2(parallaxScaleX, parallaxScaleY); }
    void setParallaxX(float v) { parallaxScaleX = v; }
    void setParallaxY(float v) { parallaxScaleY = v; }

    ax::Vec2 getOffset() const;
private:
    float parallaxScaleX = 1.f;
    float parallaxScaleY = 1.f;

    StageCameraNode *virtualCamera = nullptr;
};

#endif //PARALLAXCOMPONENT_HPP
