//
// Created by limen on 2024/8/1.
//

#ifndef STAGETESTSCENE_HPP
#define STAGETESTSCENE_HPP

#include <axmol.h>

struct KgtStage;

constexpr int stageWidth = 640 * 2, stageHeight = 480 * 2;

class StageTestScene : public ax::Scene {
public:
    bool init() override;

    void update(float delta) override;

    void loadStage(int stageNo);
    void unloadStage();
protected:
    void updatePositionByCameraPos();
private:
    KgtStage *stage = nullptr;
    ax::Vec2 cameraLoc {stageWidth / 2, stageHeight / 4};
    int cameraSpeed = 5;
    int stageNo = 0;
};

#endif //STAGETESTSCENE_HPP
