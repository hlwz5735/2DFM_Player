//
// Created by limen on 2025/1/9.
//

#ifndef OPENNINGSCENE_HPP
#define OPENNINGSCENE_HPP

#include <axmol.h>

class OpenningScene : public ax::Scene {
public:
    bool init() override;
    void update(float deltaTime) override;
private:
    class GameDemo *objDemo = nullptr;
};

#endif //OPENNINGSCENE_HPP
