#include "TestPictureScene.hpp"
#include "2dfm/2dfmFileReader.hpp"
#include "2dfm/CommonResource.hpp"
#include "game/GameConfig.hpp"
#include <axmol.h>
#include <format>

#include "game/ResourcePool.hpp"
#include "SceneNavUtil.hpp"

USING_NS_AX;

bool TestPictureScene::init() {
    if (!Scene::init()) {
        return false;
    }

    const auto &vs = _director->getVisibleSize();

    auto layer = LayerColor::create(Color4B::MAGENTA);
    addChild(layer);

    sprite = utils::createInstance<Sprite>();
    sprite->setPosition(vs.width / 2, vs.height / 2);
    addChild(sprite);

    auto &gameConfig = GameConfig::getInstance();

    auto kgtFilePath = std::format("{}/{}", gameConfig.getGameBasePath(), gameConfig.getKgtFileName());
    auto kgt = ResourcePool::getInstance().loadKgtGame(kgtFilePath);

    auto stagePtr = ResourcePool::getInstance().loadStage(0);
    createTexturesForCommonResource(stagePtr.get(), 0);

    this->cr = stagePtr.get();
    // 从第一个有实际纹理的图开始展示（spriteFrames 中大量槽位可能为空图）
    picNo = firstValidPic();

    // 左上角信息：当前图像序号 + 尺寸，便于核对图像与序列是否错位
    infoLabel = Label::createWithTTF(TTFConfig("fonts/msyh.ttc", 18), "");
    infoLabel->setAnchorPoint(Vec2(0, 1));
    infoLabel->setPosition(8, vs.height - 8);
    this->addChild(infoLabel, 100);

    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = AX_CALLBACK_2(TestPictureScene::onKeyPressed, this);
    keyboardListener->onKeyReleased = AX_CALLBACK_2(TestPictureScene::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);

    scheduleUpdate();
    addBackToMainSceneButton(this);  // 底部“返回菜单”按钮
    return true;
}

void TestPictureScene::update(float delta) {
    Scene::update(delta);
    const auto t = cr->pictures.at(picNo);
    if (t == nullptr) {
        sprite->setTexture(nullptr);
        return;
    }
    if (sprite->getTexture() != t) {
        const auto &s = cr->spriteFrames.at(picNo);
        AXLOG("picNo: %d/%d, width: %d, height: %d, privateP: %c",
            picNo, cr->pictures.size(), s.width, s.height, s.hasPrivatePalette ? 'o' : 'x');
        if (infoLabel) {
            infoLabel->setString(std::format("idx: {}/{}\nframe: {}x{}  tex: {}x{}  priv:{}", picNo,
                                             cr->pictures.size(), s.width, s.height,
                                             static_cast<int>(t->getContentSizeInPixels().width),
                                             static_cast<int>(t->getContentSizeInPixels().height),
                                             s.hasPrivatePalette ? 'o' : 'x'));
        }

        sprite->setTexture(cr->pictures.at(picNo));
        const auto &sz = t->getContentSizeInPixels();
        sprite->setTextureRect({0, 0, sz.width, sz.height});
    }
}

void TestPictureScene::onKeyPressed(EventKeyboard::KeyCode code, Event *event) {
    AXLOG("onKeyPressed, keycode: %d", static_cast<int>(code));
    if (code == EventKeyboard::KeyCode::KEY_RIGHT_ARROW) {
        picNo = nextValidPic(picNo, 1);
    }
    if (code == EventKeyboard::KeyCode::KEY_LEFT_ARROW) {
        picNo = nextValidPic(picNo, -1);
    }
}

int TestPictureScene::firstValidPic() const {
    const int n = static_cast<int>(cr->pictures.size());
    for (int i = 0; i < n; ++i) {
        auto *tex = cr->pictures.at(i);
        if (tex && tex->getContentSizeInPixels().width > 0 && tex->getContentSizeInPixels().height > 0) {
            return i;
        }
    }
    return 0;
}

int TestPictureScene::nextValidPic(int current, int dir) const {
    const int n = static_cast<int>(cr->pictures.size());
    if (n <= 1) {
        return current;
    }
    for (int step = 1; step <= n; ++step) {
        int idx = (current + dir * step) % n;
        if (idx < 0) {
            idx += n;
        }
        auto *tex = cr->pictures.at(idx);
        if (tex && tex->getContentSizeInPixels().width > 0 && tex->getContentSizeInPixels().height > 0) {
            return idx;
        }
    }
    // 除当前外没有其它合法图（或当前即唯一），停留在当前
    return current;
}

void TestPictureScene::onKeyReleased(EventKeyboard::KeyCode code, Event *event) {
    AXLOG("onKeyReleased, keycode: %d", static_cast<int>(code));
}
