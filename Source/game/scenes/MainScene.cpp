#include "MainScene.hpp"
#include <axmol.h>
#include "2dfm/2dfmFileReader.hpp"
#include "EntryScene.hpp"
#include "OpenningScene.hpp"
#include "debug/StageTestScene.hpp"
#include "debug/TestAudioEngineScene.hpp"
#include "debug/TestPictureScene.hpp"
#include "debug/PlayerTestScene.hpp"
#include "engine/Input.hpp"
#include "game/GameConfig.hpp"
#include "game/GameManager.hpp"
#include "game/ResourcePool.hpp"

#include <exception>

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

    // 不再自动加载 KGT：进入菜单只显示配置路径，由“加载 KGT”菜单项手动触发加载
    const auto& gameConfig = GameConfig::getInstance();
    const auto kgtFilePath = std::format("{}/{}", gameConfig.getGameBasePath(), gameConfig.getKgtFileName());

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
    auto startY = visibleSize.height - 60;

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
                if (scene) {
                    Director::getInstance()->replaceScene(scene);
                }
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

    // 顶部状态栏：显示当前配置路径与全局加载状态（数据以 ResourcePool 为准）
    auto &pool = ResourcePool::getInstance();
    std::string stateText;
    if (auto kgt = pool.getKgtGame()) {
        stateText = (pool.getLoadedKgtPath() == kgtFilePath)
                        ? std::format("[ 已加载: {} ]", kgt->projectName)
                        : "[ 已加载其他游戏，点“加载 KGT”切换 ]";
    } else {
        stateText = "[ 未加载 ]";
    }
    statusLabel = Label::createWithTTF(TTFConfig("fonts/msyh.ttc", 15), "KGT: " + kgtFilePath + "  " + stateText);
    statusLabel->setPosition(visibleSize.width / 2, visibleSize.height - 16);
    this->addChild(statusLabel, 2);

    return true;
}

void MainScene::onEnterTransitionDidFinish() {
    Scene::onEnterTransitionDidFinish();
}

bool MainScene::loadKgtFromConfig() {
    auto &gameConfig = GameConfig::getInstance();
    auto &pool = ResourcePool::getInstance();
    try {
        const auto kgtFilePath = std::format("{}/{}", gameConfig.getGameBasePath(), gameConfig.getKgtFileName());

        // 全局已加载同一路径：直接视为就绪，不重复加载
        if (pool.isKgtLoaded() && pool.getLoadedKgtPath() == kgtFilePath) {
            AXLOGI("[MainScene] kgt already loaded: '{}'", kgtFilePath);
            if (statusLabel && pool.getKgtGame()) {
                statusLabel->setString("KGT: " + kgtFilePath + "  [ 已加载: " + pool.getKgtGame()->projectName + " ]");
            }
            return true;
        }

        AXLOGI("[MainScene] loading kgt: '{}'", kgtFilePath);
        auto kgt = pool.loadKgtGame(kgtFilePath);
        if (!kgt) {
            AXLOGE("[MainScene] Failed to load kgt game: '{}'", kgtFilePath);
            if (statusLabel) {
                statusLabel->setString("KGT: " + kgtFilePath + "  [ 加载失败 ]");
            }
            return false;
        }
        createTexturesForCommonResource(kgt.get(), 0);
        kgt->initBasicScriptInfos();

        AXLOGI("[MainScene] kgt loaded ok: '{}'", kgt->projectName);
        if (statusLabel) {
            statusLabel->setString("KGT: " + kgtFilePath + "  [ 已加载: " + kgt->projectName + " ]");
        }
        return true;
    } catch (const std::exception &e) {
        AXLOGE("[MainScene] Failed to read kgt file: {}", e.what());
        return false;
    } catch (...) {
        AXLOGE("[MainScene] Failed to read kgt file: unknown error");
        return false;
    }
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
    // 需要先加载 KGT，其余菜单项才可用
    const auto requireLoaded = [this](const std::function<ax::Scene *()> &creator) -> ax::Scene * {
        if (!ResourcePool::getInstance().isKgtLoaded()) {
            AXLOGW("[MainScene] kgt not loaded yet");
            if (this->statusLabel) {
                this->statusLabel->setString("KGT 未加载，请先点击上方“加载 KGT”");
            }
            return nullptr;
        }
        return creator();
    };

    menuItems.emplace_back("加载 KGT", [this]() -> ax::Scene * {
        this->loadKgtFromConfig();
        return nullptr;  // 加载后停留在菜单，由用户选择后续场景
    });
    menuItems.emplace_back("卸载 KGT", [this]() -> ax::Scene * {
        ResourcePool::getInstance().unloadAll();
        if (this->statusLabel) {
            this->statusLabel->setString("KGT 已卸载，点“加载 KGT”重新加载");
        }
        AXLOGI("[MainScene] kgt unloaded by user");
        return nullptr;
    });
    menuItems.emplace_back("正常游戏场景", [requireLoaded] {
        return requireLoaded([] { return utils::createInstance<OpenningScene>(); });
    });
    menuItems.emplace_back("角色测试场景", [requireLoaded] {
        return requireLoaded([] { return ax::utils::createInstance<PlayerTestScene>(); });
    });
    menuItems.emplace_back("图片测试场景", [requireLoaded] {
        return requireLoaded([] { return ax::utils::createInstance<TestPictureScene>(); });
    });
    menuItems.emplace_back("场景测试", [requireLoaded] {
        return requireLoaded([] { return ax::utils::createInstance<StageTestScene>(); });
    });
    menuItems.emplace_back("AudioEngine 测试", [requireLoaded] {
        return requireLoaded([] { return ax::utils::createInstance<TestAudioEngineScene>(); });
    });
    menuItems.emplace_back("Select KGT File", []() -> ax::Scene * { return ax::utils::createInstance<EntryScene>(); });
}

void MainScene::scrollViewDidScroll(extension::ScrollView *view) {
    ScrollViewDelegate::scrollViewDidScroll(view);
}

void MainScene::scrollViewDidZoom(extension::ScrollView *view) {
    ScrollViewDelegate::scrollViewDidZoom(view);
}
