#include "MainScene.hpp"
#include <axmol.h>
#include "2dfm/2dfmFileReader.hpp"
#include "OpenningScene.hpp"
#include "debug/StageTestScene.hpp"
#include "debug/TestAudioEngineScene.hpp"
#include "debug/TestPictureScene.hpp"
#include "engine/Input.hpp"
#include "game/GameConfig.hpp"
#include "game/GameManager.hpp"

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
    // 此处开始是正式加载逻辑
    auto &gameConfig = GameConfig::getInstance();
    gameConfig.readAndInit();
    try {
        auto kgt = readKgtFile();
        createTexturesForCommonResource(kgt, 0);
        kgt->initBasicScriptInfos();
        GameManager::getInstance().init();
        GameManager::getInstance().setKgtGame(kgt);
    } catch (...) {
        AXLOGE("Failed to read kgt file");
        return false;
    }

    this->initDebugScenes();

    const auto safeArea = _director->getSafeAreaRect();
    const auto visibleSize = _director->getVisibleSize();
    const auto safeOrigin = safeArea.origin;

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

    TTFConfig ttfConfig("fonts/msyh.ttc", 20);
    auto startY = visibleSize.height - 30;

    auto layer = utils::createInstance<Layer>();
    for (auto it = menuItems.begin(); it != menuItems.end(); ++it) {
        const auto &title = it->first;
        const auto &callback = it->second;
        const auto label = Label::createWithTTF(ttfConfig, title);
        label->setPosition(visibleSize.width / 2, startY);
        layer->addChild(label);

        auto listener = EventListenerTouchOneByOne::create();
        listener->setSwallowTouches(true);  // 设置吞没事件
        listener->onTouchBegan = [callback](Touch *t, Event *e) {
            // 获取事件所绑定的 target
            const auto target = dynamic_cast<Label *>(e->getCurrentTarget());
            // 获取当前点击点所在相对按钮的位置坐标
            const Vec2 locationInNode = target->convertToNodeSpace(t->getLocation());
            const Size s = target->getContentSize();
            const Rect rect = Rect(0, 0, s.width, s.height);

            // 点击范围判断检测
            if (rect.containsPoint(locationInNode)) {
                auto scene = callback();
                Director::getInstance()->replaceScene(scene);
                return true;
            }
            return false;
        };

        _director->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, label);
        startY -= 40;
    }

    auto scrollView = extension::ScrollView::create(Size(visibleSize.width, visibleSize.height), layer);
    scrollView->setDelegate(this);
    this->addChild(scrollView);

    return true;
}

void MainScene::onEnterTransitionDidFinish() {
    Scene::onEnterTransitionDidFinish();
    const auto kgt = GameManager::getInstance().getKgtGame();
    if (kgt == nullptr) {
        AXLOGE("KGT is null");
        return;
    }

    // auto openDemoName =
    // std::format("{}/{}.demo", GameConfig::getInstance().getGameBasePath(), kgt->getOpeningDemoName());
    // const auto openDemoScene =
    //     utils::createInstance<DemoScene>(&DemoScene::initWithFile, openDemoName, DemoScene::DemoType::OPENING);
    // _director->replaceScene(openDemoScene);
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

void MainScene::initDebugScenes() {
    menuItems.emplace_back("正常游戏场景", [] { return utils::createInstance<OpenningScene>(); });
    menuItems.emplace_back("图片测试场景", []() { return ax::utils::createInstance<TestPictureScene>(); });
    menuItems.emplace_back("场景测试", []() { return ax::utils::createInstance<StageTestScene>(); });
    menuItems.emplace_back("AudioEngine 测试", []() { return ax::utils::createInstance<TestAudioEngineScene>(); });
}

void MainScene::scrollViewDidScroll(extension::ScrollView *view) {
    ScrollViewDelegate::scrollViewDidScroll(view);
}

void MainScene::scrollViewDidZoom(extension::ScrollView *view) {
    ScrollViewDelegate::scrollViewDidZoom(view);
}
