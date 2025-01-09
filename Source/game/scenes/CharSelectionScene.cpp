//
// Created by limen on 2025/1/9.
//

#include "CharSelectionScene.hpp"

#include "2dfm/KgtGame.hpp"
#include "engine/Input.hpp"
#include "engine/KgtNode.hpp"
#include "game/GameDemo.hpp"
#include "game/GameManager.hpp"
#include "game/KgtScriptInterceptor.hpp"

USING_NS_AX;

bool CharSelectionScene::init() {
    if (!Scene::init()) {
        return false;
    }
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
        if (auto interceptor =
            dynamic_cast<KgtScriptInterceptor *>(objP1Cursor->getComponent("GameScriptInterceptor"))
        ) {
            auto kgtGame = GameManager::getInstance().getKgtGame();
            interceptor->initRunningScript(kgtGame->player1CharSelConfirmCursorScriptId);
        }
    }
}

void CharSelectionScene::addCursorObj(int playerNo) {
    const auto kgtGame = GameManager::getInstance().getKgtGame();

    KgtNode *pCursor = utils::createInstance<KgtNode>();
    const auto interceptor = utils::createInstance<KgtScriptInterceptor>();
    interceptor->setKgtGame(kgtGame);
    interceptor->initRunningScript(playerNo == 1 ? kgtGame->player1CharSelCursorScriptId
                                                 : kgtGame->player2CharSelCursorScriptId);
    interceptor->setName("GameScriptInterceptor");
    pCursor->addComponent(interceptor);
    pCursor->setLogicPosition(cursorInitialPos);
    pCursor->scheduleUpdate();
    this->addChild(pCursor);

    if (playerNo == 1) {
        objP1Cursor = pCursor;
    } else {
        objP2Cursor = pCursor;
    }
}

void CharSelectionScene::moveP1CursorRight() {
    int colNo = p1Idx % columnCount;
    if (colNo + 1 == columnCount) {
        p1Idx -= colNo;
    } else {
        ++p1Idx;
    }
}

void CharSelectionScene::moveP1CursorLeft() {
    int colNo = p1Idx % columnCount;
    if (colNo == 0) {
        p1Idx += (columnCount - 1);
    } else {
        --p1Idx;
    }
}

void CharSelectionScene::moveP1CursorUp() {
    int rowNo = p1Idx / columnCount;
    int colNo = p1Idx % columnCount;

    if (rowNo == 0) {
        p1Idx = (rowCount - 1) * columnCount + colNo;
    } else {
        p1Idx = (rowNo - 1) * columnCount + colNo;
    }
}

void CharSelectionScene::moveP1CursorDown() {
    int rowNo = p1Idx / columnCount;
    int colNo = p1Idx % columnCount;

    if (rowNo + 1 == rowCount) {
        p1Idx = colNo;
    } else {
        p1Idx = (rowNo + 1) * columnCount + colNo;
    }
}
