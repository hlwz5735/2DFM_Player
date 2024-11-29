//
// Created by limen on 2024/8/1.
//

#ifndef STAGETESTSCENE_HPP
#define STAGETESTSCENE_HPP

#include <axmol.h>

struct KgtStage;

class StageTestScene : public ax::Scene {
public:
    bool init() override;

    void update(float delta) override;

    void loadStage(int stageNo);
    void unloadStage();
private:
    KgtStage *stage = nullptr;
    class StageCameraNode *cameraNode = nullptr;
    int stageNo = 0;
    std::vector<ax::Node*> scriptNodes;
};

#endif //STAGETESTSCENE_HPP
