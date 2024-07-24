#pragma once

#include "../base/Component.hpp"

class CommonResource;

class TestComponent : public Component {
public:
    TestComponent(Node *owner, int updateOrder = 10);
    void update(float deltaTime) override;

    void setCommonResource(CommonResource *res) { cr = res; }

private:
    CommonResource *cr = nullptr;
    class SpriteComponent *sprite = nullptr;
    int picNo = 0;
};
