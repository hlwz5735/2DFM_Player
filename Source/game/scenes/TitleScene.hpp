//
// Created by limen on 2025/1/9.
//

#ifndef TITLESCENE_HPP
#define TITLESCENE_HPP

#include <axmol.h>

class GameDemo;

class TitleScene final : public ax::Scene {
public:
    bool init() override;
    void update(float deltaTime) override;
private:
    GameDemo *objDemo = nullptr;
    int cursorIdx = -1;
    class KgtNode *cursorNode = nullptr;

    void moveCursorNext();
    void moveCursorPrev();
};

#endif //TITLESCENE_HPP
