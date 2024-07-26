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
    if (!texture) {
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

    auto pos = getPosition() + Vector2(texture->getWidth() / 2, -texture->getHeight() / 2) + Vector2(offset.x, wh - offset.y);
//    auto worldTransform = scaleMat * Matrix4::createTranslation(Vector3(pos.x, wh - (pos.y + texture->getHeight()), 0));
    auto worldTransform = scaleMat * Matrix4::createTranslation(pos) * viewMat;
    if (texture) {
        texture->setActive();
    }
    // 设置 uniform 参数
    shader->setMatrixUniform("uM", worldTransform);
    shader->setFloatUniform("uTransparency", opacity);

    // 渲染图元（三角形），顶点数为6， 顶点下标类型为无符号整型，最后一个参数则总是使用空指针
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    /*if (opacity == 0) {
        return;
    }
    int x = static_cast<int>(owner->getPosition().x);
    int y = static_cast<int>(owner->getPosition().y);
    int w = texture->getWidth();
    int h = texture->getHeight();
    SDL_Rect sr { 0, 0, w, h };

    int dx = origin == Origin::TOP_LEFT ? x : x - w / 2;
    int dy = origin == Origin::TOP_LEFT ? y : y - h;
    SDL_Rect dr { dx + static_cast<int>(offset.x), dy + static_cast<int>(offset.y), w, h };

    int flip = SDL_FLIP_NONE;
    if (flipX) {
        flip |= SDL_FLIP_HORIZONTAL;
    }
    if (flipY) {
        flip |= SDL_FLIP_VERTICAL;
    }
    SDL_RenderCopyEx(renderer->sdlRenderer, texture->getSdlTexture(), &sr, &dr, 0, nullptr, static_cast<SDL_RendererFlip>(flip));*/
}
