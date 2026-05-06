#include "EntryScene.hpp"
#include "MainScene.hpp"
#include "engine/NativeFileDialog.hpp"
#include "game/GameConfig.hpp"

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

    // Title: "2DFM Player"
    TTFConfig titleConfig("fonts/msyh.ttc", 36);
    auto titleLabel = Label::createWithTTF(titleConfig, "2DFM Player");
    titleLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 80));
    this->addChild(titleLabel);

    // "Open KGT" button
    TTFConfig btnConfig("fonts/msyh.ttc", 24);
    openBtnLabel = Label::createWithTTF(btnConfig, "[ Open KGT ]");
    openBtnLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + 20));
    this->addChild(openBtnLabel);

    // "Remember next time" checkbox toggle
    TTFConfig checkConfig("fonts/msyh.ttc", 18);
    checkboxLabel = Label::createWithTTF(checkConfig, "☐ 下次记住");
    checkboxLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 40));
    this->addChild(checkboxLabel);

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
            if (rememberChecked) {
                checkboxLabel->setString("· 下次记住");
            } else {
                checkboxLabel->setString("☐ 下次记住");
            }
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
    std::string basePath;
    std::string fileName;
    auto lastSlash = fullPath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        basePath = fullPath.substr(0, lastSlash);
        fileName = fullPath.substr(lastSlash + 1);
    } else {
        basePath = ".";
        fileName = fullPath;
    }

    auto& config = GameConfig::getInstance();
    config.setGameBasePath(basePath);
    config.setKgtFileName(fileName);

    if (rememberChecked) {
        config.setRememberKgtPath(true);
        config.save();
    }

    _director->replaceScene(utils::createInstance<MainScene>());
}
