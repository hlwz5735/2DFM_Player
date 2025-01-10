//
// Created by limen on 2025/1/9.
//

#ifndef CHARSELECTIONSCENE_HPP
#define CHARSELECTIONSCENE_HPP

#include <axmol.h>

class KgtNode;
class PlayerNode;
class KgtPlayer;

class CharSelectionScene : public ax::Scene {
public:
    bool init() override;
    void update(float delta) override;
    void onExit() override;
private:
    void addCursorObj(int playerNo);
    void addPlayerShowcaseObj(int playerNo);

    void moveP1CursorRight();
    void moveP1CursorLeft();
    void moveP1CursorUp();
    void moveP1CursorDown();
    /// 光标移动后的回调
    void afterCursorMove(int playerNo);

    KgtPlayer *readPlayer(int playerId);

    class GameDemo *objDemo = nullptr;

    int p1Idx = 0;

    KgtNode *objP1Cursor = nullptr;
    KgtNode *objP2Cursor = nullptr;
    PlayerNode *objP1Showcase = nullptr;

    int rowCount = 0;
    int columnCount = 0;
    ax::Vec2 boxSize = ax::Vec2::ZERO;
    ax::Vec2 cursorInitialPos = ax::Vec2::ZERO;

    /// 缓存的已读的玩家数
    std::vector<KgtPlayer *> cachedReadPlayers;
};

#endif //CHARSELECTIONSCENE_HPP
