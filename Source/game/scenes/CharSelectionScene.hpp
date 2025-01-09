//
// Created by limen on 2025/1/9.
//

#ifndef CHARSELECTIONSCENE_HPP
#define CHARSELECTIONSCENE_HPP

#include <axmol.h>

class CharSelectionScene : public ax::Scene {
public:
    bool init() override;
    void update(float delta) override;
private:
    class GameDemo *objDemo = nullptr;
};

#endif //CHARSELECTIONSCENE_HPP
