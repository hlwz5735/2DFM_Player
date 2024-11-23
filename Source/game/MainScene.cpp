/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 Copyright (c) 2019-present Axmol Engine contributors (see AUTHORS.md).

 https://axmol.dev/

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "MainScene.hpp"

#include "2dfm/2dfmFileReader.hpp"
#include "DemoScene.hpp"
#include "DemoScriptInterceptor.hpp"
#include "GameManager.hpp"
#include "MoveComponent.hpp"

USING_NS_AX;

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char *filename) {
    printf("Error while loading: %s\n", filename);
    printf(
        "Depending on how you compiled you might have to add 'Content/' in front of filenames in "
        "MainScene.cpp\n");
}

// on "init" you need to initialize your instance
bool MainScene::init() {
    //////////////////////////////
    // 1. super init first
    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = _director->getVisibleSize();
    auto origin = _director->getVisibleOrigin();
    auto safeArea = _director->getSafeAreaRect();
    auto safeOrigin = safeArea.origin;
    GameConfig::getInstance().readAndInit();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create("CloseNormal.png", "CloseSelected.png",
                                           AX_CALLBACK_1(MainScene::menuCloseCallback, this));

    if (closeItem == nullptr || closeItem->getContentSize().width <= 0 || closeItem->getContentSize().height <= 0) {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    } else {
        float x = safeOrigin.x + safeArea.size.width - closeItem->getContentSize().width / 2;
        float y = safeOrigin.y + closeItem->getContentSize().height / 2;
        closeItem->setPosition(Vec2(x, y));
    }

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...
    const auto &gameConfig = GameConfig::getInstance();
    auto kgtFilePath = std::format("{}/{}", gameConfig.getGameBasePath(), gameConfig.getKgtFileName());
    try {
        auto kgt = readKgtFile(kgtFilePath);
        createTexturesForCommonResource(kgt, 0);
        GameManager::getInstance().setKgtGame(kgt);
    } catch (...) {
        AXLOGE("Failed to read kgt file");
        return false;
    }

    // scheduleUpdate() is required to ensure update(float) is called on every loop
    scheduleUpdate();

    return true;
}

void MainScene::update(float delta) {
    const auto kgt = GameManager::getInstance().getKgtGame();
    if (kgt == nullptr) {
        AXLOGE("KGT is null");
        _gameState = GameState::end;
    }
    switch (_gameState) {
    case GameState::init: {
        _gameState = GameState::update;
        auto openDemoName = std::format("{}/{}.demo",
            GameConfig::getInstance().getGameBasePath(), kgt->getOpeningDemoName());
        const auto openDemoScene = utils::createInstance<DemoScene>(&DemoScene::initWithFile, openDemoName);
        _director->replaceScene(openDemoScene);
        break;
    }
    case GameState::end:
        // CleanUpMyCrap();
        menuCloseCallback(this);
        break;
    default:
        break;
    } //switch
}

void MainScene::menuCloseCallback(Object *sender) {
    // Close the axmol game scene and quit the application
    _director->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use
     * _director->end() as given above,instead trigger a custom event created in RootViewController.mm
     * as below*/

    // EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
}

void MainScene::onExit() {
    GameConfig::getInstance().save();
    Scene::onExit();
}