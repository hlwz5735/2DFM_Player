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

enum class BlendType {
    NORMAL = 0,
    ADD,
    MINUS,
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
    BlendType getBlendType() const { return blendType; }
    void setBlendType(BlendType blendType) { SpriteComponent::blendType = blendType; }
    const Vector3 &getBlendColor() const { return blendColor; }
    void setBlendColor(const Vector3 &blendColor) { SpriteComponent::blendColor = blendColor; }

private:
    bool flipX = false;
    bool flipY = false;

    int zOrder = 100;

    /// 原点位置
    Origin origin = Origin::TOP_LEFT;
    /// 混合模式
    BlendType blendType = BlendType::NORMAL;
    /// 透明度
    float opacity = 1.f;
    /// 混合颜色
    Vector3 blendColor = Vector3::ZERO;

    Vector2 offset = Vector2::ZERO;
    Texture *texture = nullptr;
};
