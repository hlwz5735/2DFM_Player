#include "TestAudioEngineScene.hpp"
#include "2dfm/2dfmFileReader.hpp"
#include "2dfm/CommonResource.hpp"
#include "game/GameConfig.hpp"
#include "engine/MyString.hpp"
#include <axmol.h>
#include <AudioEngine.h>

USING_NS_AX;

bool TestAudioEngineScene::init() {
    if (!Scene::init()) {
        return false;
    }

    const auto &visibleSize = _director->getVisibleSize();

    auto layer = LayerColor::create(Color4B::BLACK);
    addChild(layer);

    TTFConfig ttfConfig("fonts/msyh.ttc", 20);
    auto startX = visibleSize.width / 2;
    auto startY = visibleSize.height - 30;
    label = Label::createWithTTF(ttfConfig, "--声音信息--");
    label->setPosition(startX, startY);
    layer->addChild(label);

    GameConfig gameConfig;
    gameConfig.readAndInit();

    auto kgtFilePath = std::format("{}/{}", gameConfig.getGameBasePath(), gameConfig.getKgtFileName());
    auto kgt = readKgtFile(kgtFilePath);
    this->cr = kgt;

    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = AX_CALLBACK_2(TestAudioEngineScene::onKeyPressed, this);
    keyboardListener->onKeyReleased = AX_CALLBACK_2(TestAudioEngineScene::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithFixedPriority(keyboardListener, 11);

    scheduleUpdate();
    return true;
}

void TestAudioEngineScene::update(float delta) {
    Scene::update(delta);
    const auto s = cr->sounds_.at(soundNo);

    if (s == nullptr) {
        if (playingSound != nullptr) {
            AudioEngine::stopAll();
            playingSound = nullptr;
        }
        return;
    }

    std::string soundPath = std::format("kgt/0/sound/{}.wav", soundNo);
    AXLOG("Loading: %s", soundPath.c_str());
    if (playingSound != s) {
        AXLOG("Playing soundNo: %d/%d", soundNo, cr->sounds_.size());
        label->setString(std::format("{}/{}, {}, {}", soundNo, cr->sounds_.size(), gbkToUtf8(s->header.name), s->header.size));
        AudioEngine::play2d(soundPath);
        playingSound = s;
    }
}

void TestAudioEngineScene::onKeyPressed(EventKeyboard::KeyCode code, Event *event) {
    AXLOG("onKeyPressed, keycode: %d", static_cast<int>(code));
    if (code == EventKeyboard::KeyCode::KEY_RIGHT_ARROW) {
        soundNo += 1;
        if (soundNo >= cr->sounds_.size()) {
            soundNo = 0;
        }
    }
    if (code == EventKeyboard::KeyCode::KEY_LEFT_ARROW) {
        if (soundNo <= 0) {
            soundNo = cr->sounds_.size();
        }
        soundNo -= 1;
    }
}

void TestAudioEngineScene::onKeyReleased(EventKeyboard::KeyCode code, Event *event) {
    AXLOG("onKeyReleased, keycode: %d", static_cast<int>(code));
}
