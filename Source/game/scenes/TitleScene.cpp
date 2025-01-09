//
// Created by limen on 2025/1/9.
//

#include "TitleScene.hpp"

#include "CharSelectionScene.hpp"
#include "engine/Input.hpp"
#include "engine/KgtNode.hpp"
#include "game/GameConfig.hpp"
#include "game/GameDemo.hpp"
#include "game/GameManager.hpp"
#include "game/KgtScriptInterceptor.hpp"

USING_NS_AX;

bool TitleScene::init() {
    if (!Scene::init()) {
        return false;
    }
    auto kgtGame = GameManager::getInstance().getKgtGame();

    objDemo = utils::createInstance<GameDemo>();
    objDemo->load(static_cast<int>(kgtGame->demoConfig.titleDemoId) - 1);
    addChild(objDemo);

    cursorNode = utils::createInstance<KgtNode>();
    const auto interceptor = utils::createInstance<KgtScriptInterceptor>();
    interceptor->setKgtGame(kgtGame);
    interceptor->initRunningScript(kgtGame->titleCursorScriptId);
    cursorNode->addComponent(interceptor);
    cursorNode->setLogicPosition(kgtGame->getTitleStoryModePos());
    cursorNode->scheduleUpdate();
    this->addChild(cursorNode);

    if (kgtGame->projectBaseConfig.enableStoryMode) {
        cursorIdx = 0;
    } else if (kgtGame->projectBaseConfig.enable1V1Mode) {
        cursorIdx = 1;
    } else if (kgtGame->projectBaseConfig.enableTeamMode) {
        cursorIdx = 2;
    }

    scheduleUpdate();

    return true;
}

void TitleScene::update(float deltaTime) {
    Scene::update(deltaTime);
    if (Input::getInstance().isButtonDown(Input::GameButton::D_RIGHT)) {
        moveCursorNext();
    } else if (Input::getInstance().isButtonDown(Input::GameButton::D_LEFT)) {
        moveCursorPrev();
    }
    const auto kgtGame = GameManager::getInstance().getKgtGame();
    switch (cursorIdx) {
    case 0:
        cursorNode->setLogicPosition(kgtGame->getTitleStoryModePos());
        break;
    case 1:
        cursorNode->setLogicPosition(kgtGame->getTitlePvpModePos());
        break;
    case 2:
        cursorNode->setLogicPosition(kgtGame->getTitleTeamModePos());
        break;
    default:
        break;
    }

    if (Input::getInstance().isAnyAttackButtonDown()) {
        Scene *nextScene = utils::createInstance<CharSelectionScene>();
        // switch (cursorIdx) {
        // case 0:
        //     nextScene = ;
        //     break;
        // case 1:
        //     nextScene = ;
        //     break;
        // case 2:
        // default:
        //     nextScene = ;
        //     break;
        // }
        AXLOGI("准备进入角色选择DEMO：%d", cursorIdx);
        _director->replaceScene(nextScene);
    }
}

void TitleScene::moveCursorNext() {
    const auto kgtGame = GameManager::getInstance().getKgtGame();

    if (cursorIdx == 0) {
        if (kgtGame->projectBaseConfig.enable1V1Mode) {
            cursorIdx = 1;
        } else if (kgtGame->projectBaseConfig.enableTeamMode) {
            cursorIdx = 2;
        }
    } else if (cursorIdx == 1) {
        if (kgtGame->projectBaseConfig.enableTeamMode) {
            cursorIdx = 2;
        } else if (kgtGame->projectBaseConfig.enableStoryMode) {
            cursorIdx = 0;
        }
    } else if (cursorIdx == 2) {
        if (kgtGame->projectBaseConfig.enableStoryMode) {
            cursorIdx = 0;
        } else if (kgtGame->projectBaseConfig.enable1V1Mode) {
            cursorIdx = 1;
        }
    }
}

void TitleScene::moveCursorPrev() {
    const auto kgtGame = GameManager::getInstance().getKgtGame();

    if (cursorIdx == 0) {
        if (kgtGame->projectBaseConfig.enableTeamMode) {
            cursorIdx = 2;
        } else if (kgtGame->projectBaseConfig.enable1V1Mode) {
            cursorIdx = 1;
        }
    } else if (cursorIdx == 1) {
        if (kgtGame->projectBaseConfig.enableStoryMode) {
            cursorIdx = 0;
        } else if (kgtGame->projectBaseConfig.enableTeamMode) {
            cursorIdx = 2;
        }
    } else if (cursorIdx == 2) {
        if (kgtGame->projectBaseConfig.enable1V1Mode) {
            cursorIdx = 1;
        } else if (kgtGame->projectBaseConfig.enableStoryMode) {
            cursorIdx = 0;
        }
    }
}

