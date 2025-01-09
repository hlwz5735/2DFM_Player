//
// Created by limen on 2025/1/9.
//

#ifndef CHARSELECTIONSCENE_HPP
#define CHARSELECTIONSCENE_HPP

#include <axmol.h>

class KgtNode;

class CharSelectionScene : public ax::Scene {
public:
    bool init() override;
    void update(float delta) override;

private:
    void addCursorObj(int playerNo);

    void moveP1CursorRight();
    void moveP1CursorLeft();
    void moveP1CursorUp();
    void moveP1CursorDown();

    class GameDemo *objDemo = nullptr;
    KgtNode *objP1Cursor = nullptr;
    int p1Idx = 0;

    KgtNode *objP2Cursor = nullptr;

    int rowCount = 0;
    int columnCount = 0;
    ax::Vec2 boxSize = ax::Vec2::ZERO;
    ax::Vec2 cursorInitialPos = ax::Vec2::ZERO;
};

#endif //CHARSELECTIONSCENE_HPP
