#pragma once

#include "../base/Component.hpp"

class TestComponent : public Component {
public:
    TestComponent(Node *owner, int updateOrder = 10);
    void update(float deltaTime) override;

    void setKgtGame(class KgtGame *game) { kgtGame = game; }
private:
    class KgtGame *kgtGame = nullptr;
    class SpriteComponent *sprite = nullptr;
    int textureId = 0;
};
