//
// Created by limen on 2024/11/21.
//

#include "DemoScene.hpp"
#include "2dfm/2dfmFileReader.hpp"
#include "DemoScriptInterceptor.hpp"
#include "GameConfig.hpp"
#include "GameManager.hpp"
#include "KgtScriptInterceptor.hpp"
#include "MoveComponent.hpp"
#include "engine/AudioSystem.hpp"
#include "engine/Input.hpp"
#include "engine/KgtNode.hpp"

USING_NS_AX;

bool DemoScene::initWithFile(std::string_view filePath, DemoType demoType) {
    if (!Scene::init()) {
        return false;
    }

    this->demoType = demoType;

    KgtDemo *demo = readDemoFile(filePath);
    createTexturesForCommonResource(demo, 0);

    for (int i = 1; i < demo->scripts.size(); ++i) {
        auto scriptNode = utils::createInstance<KgtNode>();
        scriptNode->setLogicPosition(Vec2::ZERO);

        const auto interceptor = utils::createInstance<DemoScriptInterceptor>();
        interceptor->setDemoData(demo);
        interceptor->setRunningScript(i);
        scriptNode->addComponent(interceptor);
        scriptNode->scheduleUpdate();

        this->addChild(scriptNode);
        scriptNodes.emplace_back(scriptNode);
    }

    scheduleUpdate();

    if (demoType == DemoType::TITLE) {
        return initTitle();
    }
    return true;
}
void DemoScene::update(float deltaTime) {
    Scene::update(deltaTime);
    switch (demoType) {
    case DemoType::OPENING:
        updateOpening();
        break;
    case DemoType::TITLE:
        updateTitle();
        break;
    default:
        break;
    }
}

void DemoScene::onExit() {
    AudioSystem::getInstance().stopAll();
    Scene::onExit();
}

bool DemoScene::initTitle() {
    cursorNode = utils::createInstance<KgtNode>();
    auto kgtGame = GameManager::getInstance().getKgtGame();
    const auto interceptor = utils::createInstance<KgtScriptInterceptor>();
    interceptor->setKgtGame(kgtGame);
    interceptor->setRunningScript(kgtGame->titleCursorScriptId); // TODO: 待完善
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

    return true;
}

void DemoScene::updateOpening() {
    if (Input::getInstance().isAnyAttackButtonDown()) {
        auto openDemoName = std::format("{}/{}.demo", GameConfig::getInstance().getGameBasePath(),
                                        GameManager::getInstance().getKgtGame()->getTitleDemoName());
        const auto openDemoScene =
            utils::createInstance<DemoScene>(&DemoScene::initWithFile, openDemoName, DemoType::TITLE);
        _director->replaceScene(openDemoScene);
    }
}
void DemoScene::updateTitle() {
    if (Input::getInstance().isButtonDown(Input::GameButton::D_RIGHT)) {
        titleNextMode();
    } else if (Input::getInstance().isButtonDown(Input::GameButton::D_LEFT)) {
        titlePrevMode();
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
        auto demoName = std::format("{}/{}.demo", GameConfig::getInstance().getGameBasePath(),
                                   kgtGame->getCharSelectionDemoName());
        const auto openDemoScene =
            utils::createInstance<DemoScene>(&DemoScene::initWithFile, demoName, DemoType::CHAR_SEL);
        _director->replaceScene(openDemoScene);
    }
}
void DemoScene::titleNextMode() {
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
void DemoScene::titlePrevMode() {
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
