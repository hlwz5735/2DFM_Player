//
// Created by limen on 2024/8/1.
//

#ifndef STAGETESTSCENE_HPP
#define STAGETESTSCENE_HPP

#include <2d/Scene.h>

struct KgtStage;

class StageTestScene : public ax::Scene {
public:
    bool init() override;

    void onMouseDown(ax::Event* event);
    void onMouseUp(ax::Event* event);
    void onMouseMove(ax::Event* event);

private:
    KgtStage *stage = nullptr;

    ax::Vec2 ploc = ax::Vec2::ZERO;
    ax::Vec2 loc = ax::Vec2::ZERO;
};

#endif //STAGETESTSCENE_HPP
