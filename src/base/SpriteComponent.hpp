#pragma once
#include "../math/Vector.hpp"
#include "Component.hpp"
#include "Node.hpp"

class Renderer;
class Texture;

enum class Origin {
    TOP_LEFT,
    BOTTOM_CENTER
};

class SpriteComponent : public Component {
public:
    SpriteComponent(Node *owner, int zOrder = 100);
    ~SpriteComponent();

    void draw(class Shader *shader);

    const Vector2 &getPosition() const { return owner->getPosition(); }

    const Vector2 &getOffset() const {return offset;}
    void setOffset(const Vector2 &off) { offset = off; }
    int getZOrder() const { return zOrder; }
    void setZOrder(int order) { zOrder = order; }
    Texture *getTexture() const { return texture; }
    void setTexture(Texture *tex) { texture = tex; }
    bool isFlipX() const {return flipX;}
    void setFlipX(bool flipX) {SpriteComponent::flipX = flipX;}
    bool isFlipY() const {return flipY;}
    void setFlipY(bool flipY) {SpriteComponent::flipY = flipY;}
    Origin getOrigin() const {return origin;}
    void setOrigin(Origin origin) {SpriteComponent::origin = origin;}
    float getOpacity() const {return opacity;}
    void setOpacity(float opacity) {SpriteComponent::opacity = opacity;}

private:
    bool flipX = false;
    bool flipY = false;

    int zOrder = 100;

    Origin origin = Origin::TOP_LEFT;
    float opacity = 1.f;

    Vector2 offset = Vector2::ZERO;
    Texture *texture = nullptr;
};
