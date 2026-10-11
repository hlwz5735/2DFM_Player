//
// Created by limen on 2025/1/9.
//

#include "CharSelectionScene.hpp"

#include "2dfm/CommonResource.hpp"
#include "2dfm/KgtGame.hpp"
#include "2dfm/KgtPlayer.hpp"
#include "engine/Input.hpp"
#include "engine/KgtNode.hpp"
#include "game/GameConfig.hpp"
#include "game/GameDemo.hpp"
#include "game/ResourcePool.hpp"
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
    const auto kgtGame = ResourcePool::getInstance().getKgtGame();

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

    if (Input::getInstance().isDirPressed(Input::DPadAxis::Right)) {
        moveP1CursorRight();
    } else if (Input::getInstance().isDirPressed(Input::DPadAxis::Left)) {
        moveP1CursorLeft();
    } else if (Input::getInstance().isDirPressed(Input::DPadAxis::Down)) {
        moveP1CursorDown();
    } else if (Input::getInstance().isDirPressed(Input::DPadAxis::Up)) {
        moveP1CursorUp();
    }

    int rowNo = p1Idx / columnCount;
    int colNo = p1Idx % columnCount;

    objP1Cursor->setLogicPosition(cursorInitialPos + Vec2(colNo * boxSize.width, rowNo * boxSize.height));

    if (Input::getInstance().isAnyAttackPressed()) {
        auto kgtGame = ResourcePool::getInstance().getKgtGame();
        if (!kgtGame->isPlayerInStoryMode(p1Idx)) {
            return;
        }
        if (auto interceptor = objP1Cursor->getInterceptor()) {
            interceptor->initRunningScript(kgtGame->player1CharSelConfirmCursorScriptId);
        }
    }
}
void CharSelectionScene::onExit() {
    cachedReadPlayers.clear();
    Scene::onExit();
}

void CharSelectionScene::addCursorObj(int playerNo) {
    const auto kgtGame = ResourcePool::getInstance().getKgtGame();

    KgtNode *pCursor = utils::createInstance<KgtNode>();
    const auto interceptor = utils::createInstance<KgtScriptInterceptor>();
    interceptor->setKgtGame(kgtGame.get());
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

    auto kgtGame = ResourcePool::getInstance().getKgtGame();
    if (playerNo == 1) {
        pNode->setLogicPosition(kgtGame->charSelectConfig.player1PortraitPos);
        // 展示用的角色没有状态机：段落脚本一播完（含末尾隐形的“完”）它就会自行回收，
        // 所以这里只按名字标记，不保存裸指针（见 getP1Showcase）
        pNode->setName("P1Showcase");
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
    auto kgtGame = ResourcePool::getInstance().getKgtGame();
    if (playerNo == 1) {
        const auto objP1Showcase = getP1Showcase();
        if (objP1Showcase == nullptr) {
            return;
        }
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

PlayerNode *CharSelectionScene::getP1Showcase() {
    // 展示节点没有状态机，段落脚本播完会自行回收，裸指针会悬空；
    // 所以每次都按名字从场景里现取，取不到就是已经被回收了。
    return dynamic_cast<PlayerNode *>(this->getChildByName("P1Showcase"));
}

std::shared_ptr<KgtPlayer> CharSelectionScene::readPlayer(int playerId) {
    auto kgtGame = ResourcePool::getInstance().getKgtGame();
    auto playerName = kgtGame->playerNames[playerId];
    auto i = std::find_if(cachedReadPlayers.begin(), cachedReadPlayers.end(), [&playerName](const std::shared_ptr<KgtPlayer> &p) {
        return p->playerName == playerName;
    });
    if (i != cachedReadPlayers.end()) {
        return *i;
    }

    // TODO: 目前尚未使用 CachedPlayerCount 限制读取玩家数据的数量
    auto playerPtr = ResourcePool::getInstance().loadPlayer(playerId);
    if (!playerPtr) return nullptr;
    playerPtr->initBasicScriptInfos();
    createTexturesForCommonResource(playerPtr.get(), 0);
    cachedReadPlayers.emplace_back(playerPtr);
    return playerPtr;
}
