#pragma once
#include "../math/Vector.hpp"
#include "Component.hpp"
#include "Node.hpp"

class Renderer;
class Texture;

enum Origin {
    TOP_LEFT,
    BOTTOM_CENTER
};

class SpriteComponent : public Component {
public:
    SpriteComponent(Node *owner, int updateOrder = 100);
    ~SpriteComponent();

    void draw(Renderer *renderer);

    const Vector2 &getPosition() const { return owner->getPosition(); }
    void setOffset(const Vector2 &off) { offset = off; }
    int getZOrder() const { return zOrder; }
    void setZOrder(int order) { zOrder = order; }
    Texture *getTexture() const { return texture; }
    void setTexture(Texture *tex) { texture = tex; }
private:
    bool flipX = false;
    bool flipY = false;

    int zOrder = 100;

    Origin origin = TOP_LEFT;

    Vector2 offset = Vector2::ZERO;
    Texture *texture = nullptr;
};
