#include "MainScene.hpp"
#include <axmol.h>
#include "2dfm/2dfmFileReader.hpp"
#include "OpenningScene.hpp"
#include "debug/StageTestScene.hpp"
#include "debug/TestPictureScene.hpp"
#include "engine/Input.hpp"
#include "game/GameConfig.hpp"
#include "game/GameManager.hpp"
#include "ui/axmol-ui.h"

USING_NS_AX;

namespace {
class OpenKgtFileLayer : public Layer {
public:
    bool init() override;
};

bool OpenKgtFileLayer::init() {
    if (!Layer::init()) {
        return false;
    }

    const auto visibleSize = _director->getVisibleSize();
    TTFConfig ttfConfig("fonts/msyh.ttc", 20);

    auto label = Label::createWithTTF(ttfConfig, "Open Kgt File");

    auto inputField = ui::TextFieldEx::create("Please enter...", "fonts/msyh.ttc", 20);
    inputField->setString("./Game.kgt");
    inputField->setContentSize(inputField->getRenderLabel()->getContentSize());

    // 为 inputField 添加触摸事件监听器
    inputField->enableIME(nullptr);
    inputField->setEnabled(true);

    label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + 50));
    inputField->setPosition(Vec2(12, visibleSize.height / 2 - 50));

    this->addChild(label);
    this->addChild(inputField);

    auto btn = ui::Button::create("CloseNormal.png", "CloseSelected.png");
    btn->addClickEventListener([this, inputField](Object* sender) {
        // 处理按钮点击事件，例如加载 kgt 文件
        std::string kgtPath = std::string(inputField->getString());
        AXLOG("Selected kgt file: %s", kgtPath.c_str());
        try {
            auto kgt = readKgtFile(kgtPath);
            createTexturesForCommonResource(kgt, 0);
            kgt->initBasicScriptInfos();
            GameManager::getInstance().init();
            GameManager::getInstance().setKgtGame(kgt);

            auto parent = dynamic_cast<MainScene *>(this->getParent());
            parent->chuantongxiangyan();
        } catch (...) {
            AXLOGE("Failed to read kgt file");
        }
    });
    btn->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 100));
    this->addChild(btn);
        
    return true;
}
}  // namespace

static void initDebugScenes(std::vector<std::pair<std::string, std::function<ax::Scene *()>>> &menuItems) {
    menuItems.emplace_back("正常游戏场景", [] { return utils::createInstance<OpenningScene>(); });
    menuItems.emplace_back("图片测试场景", []() { return ax::utils::createInstance<TestPictureScene>(); });
    menuItems.emplace_back("场景测试", []() { return ax::utils::createInstance<StageTestScene>(); });
}

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

    initDebugScenes(this->menuItems);

    // 此处开始是正式加载逻辑
    GameConfig::getInstance().readAndInit();
    const auto &gameConfig = GameConfig::getInstance();
    auto kgtFilePath = std::format("{}/{}", gameConfig.getGameBasePath(), gameConfig.getKgtFileName());
    try {
        auto kgt = readKgtFile(kgtFilePath);
        createTexturesForCommonResource(kgt, 0);
        kgt->initBasicScriptInfos();
        GameManager::getInstance().init();
        GameManager::getInstance().setKgtGame(kgt);

        this->initMenuLayer();
    } catch (...) {
        AXLOGE("Failed to read kgt file");

        auto newLayer = utils::createInstance<OpenKgtFileLayer>();
        if (!newLayer) {
            return false;
        }
        this->currentLayer = newLayer;
        this->addChild(newLayer);
    }

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

void MainScene::scrollViewDidScroll(extension::ScrollView *view) {
    ScrollViewDelegate::scrollViewDidScroll(view);
}

void MainScene::scrollViewDidZoom(extension::ScrollView *view) {
    ScrollViewDelegate::scrollViewDidZoom(view);
}
void MainScene::initMenuLayer() {
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
    this->currentLayer = scrollView;
}
