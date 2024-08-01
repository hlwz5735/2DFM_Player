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
private:
    KgtStage *stage = nullptr;
};

#endif //STAGETESTSCENE_HPP
