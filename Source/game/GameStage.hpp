//
// Created by limen on 2025/1/9.
//

#ifndef GAMESTAGE_HPP
#define GAMESTAGE_HPP

#include <axmol.h>

struct KgtStage;

class GameStage : public ax::Node {
public:
    bool init() override;
    void update(float delta) override;
    void onExit() override;

    void load(int stageNo);
    void unload();
private:
    KgtStage *stage = nullptr;
    class StageCameraNode *cameraNode = nullptr;
    std::vector<ax::Node*> scriptNodes;
};

#endif //GAMESTAGE_HPP
