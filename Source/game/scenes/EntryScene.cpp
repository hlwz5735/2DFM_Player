#include "EntryScene.hpp"
#include "MainScene.hpp"
#include "2dfm/2dfmFileReader.hpp"
#include "2dfm/KgtGame.hpp"
#include "engine/NativeFileDialog.hpp"
#include "game/GameConfig.hpp"
#include "game/ResourcePool.hpp"

USING_NS_AX;

static bool hitTestLabel(Label* label, Touch* touch) {
    Vec2 locationInNode = label->convertToNodeSpace(touch->getLocation());
    Size s = label->getContentSize();
    Rect rect(0, 0, s.width, s.height);
    return rect.containsPoint(locationInNode);
}

bool EntryScene::init() {
    if (!Scene::init()) {
        return false;
    }

    const auto visibleSize = _director->getVisibleSize();
    const auto cx = visibleSize.width / 2;

    // Title: "2DFM Player"
    TTFConfig titleConfig("fonts/msyh.ttc", 36);
    auto titleLabel = Label::createWithTTF(titleConfig, "2DFM Player");
    titleLabel->setPosition(Vec2(cx, visibleSize.height - 80));
    this->addChild(titleLabel);

    // "Open KGT" button
    TTFConfig btnConfig("fonts/msyh.ttc", 24);
    openBtnLabel = Label::createWithTTF(btnConfig, "[ Open KGT ]");
    openBtnLabel->setPosition(Vec2(cx, visibleSize.height / 2 + 90));
    this->addChild(openBtnLabel);

    // 已选择文件的路径显示（选择后出现）
    pathLabel = Label::createWithTTF(TTFConfig("fonts/msyh.ttc", 14), "");
    pathLabel->setPosition(Vec2(cx, visibleSize.height / 2 + 35));
    pathLabel->setVisible(false);
    this->addChild(pathLabel);

    // "Remember next time" checkbox toggle
    TTFConfig checkConfig("fonts/msyh.ttc", 18);
    checkboxLabel = Label::createWithTTF(checkConfig, "☐ 下次记住");
    checkboxLabel->setPosition(Vec2(cx, visibleSize.height / 2 - 15));
    this->addChild(checkboxLabel);

    // 确认加载按钮（选择文件后才出现）
    confirmBtnLabel = Label::createWithTTF(TTFConfig("fonts/msyh.ttc", 22), "[ 确认加载 ]");
    confirmBtnLabel->setPosition(Vec2(cx, visibleSize.height / 2 - 65));
    confirmBtnLabel->setVisible(false);
    this->addChild(confirmBtnLabel);

    // 返回菜单按钮：不选择文件，直接回到上一个场景
    backBtnLabel = Label::createWithTTF(TTFConfig("fonts/msyh.ttc", 22), "[ 返回菜单 ]");
    backBtnLabel->setPosition(Vec2(cx, 40));
    this->addChild(backBtnLabel);

    // Single touch listener on the scene handles all clickable elements
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [this](Touch* t, Event* e) {
        // Check "Open KGT" button
        if (hitTestLabel(openBtnLabel, t)) {
            this->onOpenKgtClicked();
            return true;
        }
        // Check checkbox
        if (hitTestLabel(checkboxLabel, t)) {
            rememberChecked = !rememberChecked;
            checkboxLabel->setString(rememberChecked ? "· 下次记住" : "☐ 下次记住");
            return true;
        }
        // 确认加载
        if (confirmBtnLabel && confirmBtnLabel->isVisible() && hitTestLabel(confirmBtnLabel, t)) {
            this->onConfirmClicked();
            return true;
        }
        // 返回菜单
        if (backBtnLabel && hitTestLabel(backBtnLabel, t)) {
            AXLOGI("[EntryScene] back to menu without selecting file");
            _director->replaceScene(utils::createInstance<MainScene>());
            return true;
        }
        return false;
    };
    _director->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

void EntryScene::onOpenKgtClicked() {
    auto path = NativeFileDialog::openFileDialog("Select KGT File", "KGT Files", "*.kgt");
    if (path.empty()) {
        return;  // User cancelled
    }
    onFileSelected(path);
}

void EntryScene::onFileSelected(const std::string& fullPath) {
    // Split full path into directory + filename
    auto lastSlash = fullPath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        pendingBasePath = fullPath.substr(0, lastSlash);
        pendingKgtName = fullPath.substr(lastSlash + 1);
    } else {
        pendingBasePath = ".";
        pendingKgtName = fullPath;
    }
    hasPendingSelection = true;

    // 暂不回写 GameConfig：等用户点击“确认加载”才真正生效
    AXLOGI("[EntryScene] selected kgt: full='{}', base='{}', name='{}'", fullPath, pendingBasePath, pendingKgtName);

    pathLabel->setString("已选择: " + fullPath);
    pathLabel->setVisible(true);
    confirmBtnLabel->setString("[ 确认加载 ]");
    confirmBtnLabel->setVisible(true);
}

void EntryScene::onConfirmClicked() {
    if (!hasPendingSelection) {
        pathLabel->setString("请先点击 Open KGT 选择文件");
        pathLabel->setVisible(true);
        return;
    }

    // 回写 GameConfig（勾选记住则一并写入配置文件）
    auto& config = GameConfig::getInstance();
    config.setGameBasePath(pendingBasePath);
    config.setKgtFileName(pendingKgtName);
    if (rememberChecked) {
        config.setRememberKgtPath(true);
        config.save();
    }
    AXLOGI("[EntryScene] confirm kgt: base='{}', name='{}', remember={}", pendingBasePath, pendingKgtName,
           rememberChecked);

    // 加载 KGT（与当前已加载不同时会自动切换），成功后才进入主菜单
    const auto fullPath = pendingBasePath + "/" + pendingKgtName;
    auto kgt = ResourcePool::getInstance().loadKgtGame(fullPath);
    if (!kgt) {
        AXLOGE("[EntryScene] load kgt failed: '{}'", fullPath);
        pathLabel->setString("[ 加载失败 ] " + fullPath);
        confirmBtnLabel->setString("[ 重试 ]");
        return;
    }
    createTexturesForCommonResource(kgt.get(), 0);
    kgt->initBasicScriptInfos();

    auto scene = utils::createInstance<MainScene>();
    if (scene == nullptr) {
        AXLOGE("[EntryScene] Failed to create MainScene");
        pathLabel->setString("[ 场景创建失败 ]");
        return;
    }
    _director->replaceScene(scene);
}
