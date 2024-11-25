#pragma once

#include <2d/Scene.h>

namespace ax {
class Sprite;
}
class CommonResource;

class TestPictureScene : public ax::Scene {
public:
    bool init() override;

    void update(float delta) override;

    // Keyboard
    void onKeyPressed(ax::EventKeyboard::KeyCode code, ax::Event *event);
    void onKeyReleased(ax::EventKeyboard::KeyCode code, ax::Event *event);
private:
    ax::Sprite *sprite;
    CommonResource *cr = nullptr;
    int picNo = 0;
};
