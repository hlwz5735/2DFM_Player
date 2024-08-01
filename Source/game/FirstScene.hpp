//
// Created by limen on 2024/8/1.
//

#ifndef FIRSTSCENE_HPP
#define FIRSTSCENE_HPP

#include <2d/Scene.h>

class FirstScene : public ax::Scene {
public:
    bool init() override;

    void onEnterTransitionDidFinish() override;
};

#endif //FIRSTSCENE_HPP
