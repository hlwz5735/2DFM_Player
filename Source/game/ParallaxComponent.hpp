//
// Created by limen on 2024/11/25.
//

#ifndef PARALLAXCOMPONENT_HPP
#define PARALLAXCOMPONENT_HPP

#include <axmol.h>

class ParallaxComponent : public ax::Component {
public:
    bool init(class StageCameraNode *cameraNode);
    void onAdd() override;
    void update(float delta) override;

    void setParallaxX(float v) { parallaxScaleX = v; }
    void setParallaxY(float v) { parallaxScaleY = v; }
private:
    float parallaxScaleX = 1.f;
    float parallaxScaleY = 1.f;

    StageCameraNode *virtualCamera = nullptr;
    ax::Vec2 originPos = ax::Vec2(0, 0);
};

#endif //PARALLAXCOMPONENT_HPP
