#pragma once

#include <axmol.h>

#include <memory>

#include "2dfm/2dfmCommon.hpp"
#include "AudioMacros.h"

namespace ax {
class Sprite;
}
class CommonResource;
struct KgtGame;

class TestAudioEngineScene : public ax::Scene {
public:
    bool init() override;

    void update(float delta) override;

    // Keyboard
    void onKeyPressed(ax::EventKeyboard::KeyCode code, ax::Event *event);
    void onKeyReleased(ax::EventKeyboard::KeyCode code, ax::Event *event);
private:
    std::shared_ptr<KgtGame> cr;
    int soundNo = 0;
    _2dfm::Sound *playingSound = nullptr;
    ax::Label *label = nullptr;
};
