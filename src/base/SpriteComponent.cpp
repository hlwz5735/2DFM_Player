#include "SpriteComponent.hpp"
#include "Game.hpp"
#include "Renderer.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include <SDL.h>

SpriteComponent::SpriteComponent(Node *owner, int zOrder): Component(owner), zOrder(zOrder) {
    auto renderer = owner->getGame()->getRenderer();
    renderer->addSprite(this);
}

SpriteComponent::~SpriteComponent() {
    auto renderer = owner->getGame()->getRenderer();
    renderer->removeSprite(this);
}

void SpriteComponent::draw(Shader *shader) {
    if (!texture || !texture->isBound()) {
        return;
    }
    if (opacity == 0) {
        return;
    }

    auto scaleMat = Matrix4::createScale(
            static_cast<float>(texture->getWidth()),
            static_cast<float>(texture->getHeight()),
            1.f);
    const auto ww = Game::getInstance()->getRenderer()->getWindowWidth();
    const auto wh = Game::getInstance()->getRenderer()->getWindowHeight();
    auto viewMat = Matrix4::createTranslation(Vector3(-ww / 2, -wh / 2, 0));

    // TODO: 其他 origin 尚未实现
    auto pos = getPosition() + Vector2(texture->getWidth() / 2, -texture->getHeight() / 2) + Vector2(offset.x, wh - offset.y);
    auto worldTransform = scaleMat * Matrix4::createTranslation(pos) * viewMat;
    if (texture) {
        texture->setActive();
    }
    // 设置 uniform 参数
    shader->setMatrixUniform("uM", worldTransform);
    shader->setVectorUniform("uBlendColor", blendColor);
    shader->setFloatUniform("uTransparency", opacity);

    // TODO: X/Y反转的代码尚未实现

    switch(blendType) {
        case BlendType::ADD:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
        case BlendType::MINUS:
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
            break;
        case BlendType::NORMAL:
        default:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
    }

    // 渲染图元（三角形），顶点数为6， 顶点下标类型为无符号整型，最后一个参数则总是使用空指针
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}
