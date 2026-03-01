#include "TestPictureScene.hpp"
#include "2dfm/2dfmFileReader.hpp"
#include "2dfm/CommonResource.hpp"
#include "game/GameConfig.hpp"
#include <axmol.h>

#include "game/GameManager.hpp"

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

    GameConfig &gameConfig = GameConfig::getInstance();
    gameConfig.readAndInit();

    auto kgt = readKgtFile();

    GameManager &gm = GameManager::getInstance();
    gm.setKgtGame(kgt);

    auto stage = readStageByNo(0);
    createTexturesForCommonResource(stage, 0);

    this->cr = stage;

    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = AX_CALLBACK_2(TestPictureScene::onKeyPressed, this);
    keyboardListener->onKeyReleased = AX_CALLBACK_2(TestPictureScene::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithFixedPriority(keyboardListener, 11);

    scheduleUpdate();
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

        sprite->setTexture(cr->pictures.at(picNo));
        const auto &sz = t->getContentSizeInPixels();
        sprite->setTextureRect({0, 0, sz.width, sz.height});
    }
}

void TestPictureScene::onKeyPressed(EventKeyboard::KeyCode code, Event *event) {
    AXLOG("onKeyPressed, keycode: %d", static_cast<int>(code));
    if (code == EventKeyboard::KeyCode::KEY_RIGHT_ARROW) {
        picNo += 1;
        if (picNo >= cr->spriteFrames.size()) {
            picNo = 0;
        }
    }
    if (code == EventKeyboard::KeyCode::KEY_LEFT_ARROW) {
        if (picNo <= 0) {
            picNo = cr->spriteFrames.size();
        }
        picNo -= 1;
    }
}

void TestPictureScene::onKeyReleased(EventKeyboard::KeyCode code, Event *event) {
    AXLOG("onKeyReleased, keycode: %d", static_cast<int>(code));
}
