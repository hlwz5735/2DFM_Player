#include "MainScene.hpp"
#include "2dfm/2dfmFileReader.hpp"
#include "DemoScene.hpp"
#include "GameManager.hpp"
#include "MoveComponent.hpp"
#include "engine/Input.hpp"

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
    if (!Scene::init()) {
        return false;
    }
    if (!Input::getInstance().init()) {
        AXLOGE("Failed to init Input system.");
        return false;
    }

    auto safeArea = _director->getSafeAreaRect();
    auto safeOrigin = safeArea.origin;
    GameConfig::getInstance().readAndInit();

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
        const auto openDemoScene = utils::createInstance<DemoScene>(
            &DemoScene::initWithFile, openDemoName, DemoScene::DemoType::OPENING);
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
