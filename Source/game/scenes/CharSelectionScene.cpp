//
// Created by limen on 2025/1/9.
//

#include "CharSelectionScene.hpp"

#include "2dfm/2dfmFileReader.hpp"
#include "2dfm/KgtGame.hpp"
#include "2dfm/KgtPlayer.hpp"
#include "engine/Input.hpp"
#include "engine/KgtNode.hpp"
#include "game/GameConfig.hpp"
#include "game/GameDemo.hpp"
#include "game/GameManager.hpp"
#include "game/KgtScriptInterceptor.hpp"
#include "game/PlayerNode.hpp"
#include "game/PlayerScriptInterceptor.hpp"

USING_NS_AX;

constexpr int CachedPlayerCount = 5;

bool CharSelectionScene::init() {
    if (!Scene::init()) {
        return false;
    }
    cachedReadPlayers.reserve(CachedPlayerCount);
    const auto kgtGame = GameManager::getInstance().getKgtGame();

    rowCount = kgtGame->charSelectConfig.rowCount;
    columnCount = kgtGame->charSelectConfig.columnCount;
    boxSize = kgtGame->charSelectConfig.playerAvatarIconSize;
    cursorInitialPos = kgtGame->charSelectConfig.selectBoxStartPos;

    objDemo = utils::createInstance<GameDemo>();
    // TODO: 暂时写死用故事模式的DEMO
    objDemo->load(kgtGame->getStoryModeCharSelectionDemoNo());
    addChild(objDemo);

    addCursorObj(1);
    addPlayerShowcaseObj(1);

    scheduleUpdate();
    return true;
}

void CharSelectionScene::update(float delta) {
    Scene::update(delta);

    if (Input::getInstance().isButtonDown(Input::GameButton::D_RIGHT)) {
        moveP1CursorRight();
    } else if (Input::getInstance().isButtonDown(Input::GameButton::D_LEFT)) {
        moveP1CursorLeft();
    } else if (Input::getInstance().isButtonDown(Input::GameButton::D_DOWN)) {
        moveP1CursorDown();
    } else if (Input::getInstance().isButtonDown(Input::GameButton::D_UP)) {
        moveP1CursorUp();
    }

    int rowNo = p1Idx / columnCount;
    int colNo = p1Idx % columnCount;

    objP1Cursor->setLogicPosition(cursorInitialPos + Vec2(colNo * boxSize.width, rowNo * boxSize.height));

    if (Input::getInstance().isAnyAttackButtonDown()) {
        auto kgtGame = GameManager::getInstance().getKgtGame();
        if (!kgtGame->isPlayerInStoryMode(p1Idx)) {
            return;
        }
        if (auto interceptor = objP1Cursor->getInterceptor()) {
            interceptor->initRunningScript(kgtGame->player1CharSelConfirmCursorScriptId);
        }
    }
}
void CharSelectionScene::onExit() {
    while (!cachedReadPlayers.empty()) {
        auto p = cachedReadPlayers.back();
        delete p;
        cachedReadPlayers.pop_back();
    }
    Scene::onExit();
}

void CharSelectionScene::addCursorObj(int playerNo) {
    const auto kgtGame = GameManager::getInstance().getKgtGame();

    KgtNode *pCursor = utils::createInstance<KgtNode>();
    const auto interceptor = utils::createInstance<KgtScriptInterceptor>();
    interceptor->setKgtGame(kgtGame);
    interceptor->initRunningScript(playerNo == 1 ? kgtGame->player1CharSelCursorScriptId
                                                 : kgtGame->player2CharSelCursorScriptId);
    pCursor->addInterceptor(interceptor);
    pCursor->setLogicPosition(cursorInitialPos);
    pCursor->scheduleUpdate();
    this->addChild(pCursor);

    if (playerNo == 1) {
        objP1Cursor = pCursor;
    } else {
        objP2Cursor = pCursor;
    }
}

void CharSelectionScene::addPlayerShowcaseObj(int playerNo) {
    auto pNode = utils::createInstance<PlayerNode>();
    const auto interceptor = utils::createInstance<PlayerScriptInterceptor>();
    // 光标默认停在第1个角色上
    auto player = this->readPlayer(0);
    if (player->portraitScriptId != 0) {
        interceptor->setPlayerData(player);
        interceptor->initRunningScript(player->portraitScriptId);
    } else {
        interceptor->setPlayerData(nullptr);
    }
    pNode->addInterceptor(interceptor);
    pNode->scheduleUpdate();
    this->addChild(pNode);

    auto kgtGame = GameManager::getInstance().getKgtGame();
    if (playerNo == 1) {
        pNode->setLogicPosition(kgtGame->charSelectConfig.player1PortraitPos);
        objP1Showcase = pNode;
    }
}

void CharSelectionScene::moveP1CursorRight() {
    int colNo = p1Idx % columnCount;
    if (colNo + 1 == columnCount) {
        p1Idx -= colNo;
    } else {
        ++p1Idx;
    }
    afterCursorMove(1);
}

void CharSelectionScene::moveP1CursorLeft() {
    int colNo = p1Idx % columnCount;
    if (colNo == 0) {
        p1Idx += (columnCount - 1);
    } else {
        --p1Idx;
    }
    afterCursorMove(1);
}

void CharSelectionScene::moveP1CursorUp() {
    int rowNo = p1Idx / columnCount;
    int colNo = p1Idx % columnCount;

    if (rowNo == 0) {
        p1Idx = (rowCount - 1) * columnCount + colNo;
    } else {
        p1Idx = (rowNo - 1) * columnCount + colNo;
    }
    afterCursorMove(1);
}

void CharSelectionScene::moveP1CursorDown() {
    int rowNo = p1Idx / columnCount;
    int colNo = p1Idx % columnCount;

    if (rowNo + 1 == rowCount) {
        p1Idx = colNo;
    } else {
        p1Idx = (rowNo + 1) * columnCount + colNo;
    }
    afterCursorMove(1);
}

void CharSelectionScene::afterCursorMove(int playerNo) {
    auto kgtGame = GameManager::getInstance().getKgtGame();
    if (playerNo == 1) {
        objP1Showcase->destroyManagedObjects();
        if (auto interceptor = dynamic_cast<PlayerScriptInterceptor *>(objP1Showcase->getInterceptor())) {
            if (kgtGame->isPlayerInStoryMode(p1Idx)) {
                auto player = readPlayer(p1Idx);
                if (player->portraitScriptId != 0) {
                    interceptor->setPlayerData(player);
                    interceptor->initRunningScript(player->portraitScriptId);
                } else {
                    interceptor->setPlayerData(nullptr);
                }
            } else {
                interceptor->setPlayerData(nullptr);
            }
        }
    }
}

KgtPlayer *CharSelectionScene::readPlayer(int playerId) {
    auto kgtGame = GameManager::getInstance().getKgtGame();
    auto playerName = kgtGame->playerNames[playerId];
    auto i = std::find_if(cachedReadPlayers.begin(), cachedReadPlayers.end(), [&playerName](const KgtPlayer *p) {
        return p->playerName == playerName;
    });
    if (i != cachedReadPlayers.end()) {
        return *i;
    }

    // TODO: 目前尚未使用 CachedPlayerCount 限制读取玩家数据的数量
    auto player = readPlayerByNo(playerId);
    player->initBasicScriptInfos();
    createTexturesForCommonResource(player, 0);
    cachedReadPlayers.emplace_back(player);
    return player;
}
