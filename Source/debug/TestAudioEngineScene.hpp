#pragma once

#include <axmol.h>

#include "2dfm/2dfmCommon.hpp"
#include "AudioMacros.h"

namespace ax {
class Sprite;
}
class CommonResource;

class TestAudioEngineScene : public ax::Scene {
public:
    bool init() override;

    void update(float delta) override;

    // Keyboard
    void onKeyPressed(ax::EventKeyboard::KeyCode code, ax::Event *event);
    void onKeyReleased(ax::EventKeyboard::KeyCode code, ax::Event *event);
private:
    CommonResource *cr = nullptr;
    int soundNo = 0;
    _2dfm::Sound *playingSound = nullptr;
    ax::Label *label = nullptr;
};
