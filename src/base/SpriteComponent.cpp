#include "SpriteComponent.hpp"
#include <SDL.h>

#include "Game.hpp"
#include "Renderer.hpp"
#include "Texture.hpp"

SpriteComponent::SpriteComponent(Node *owner, int updateOrder): Component(owner, updateOrder) {
    auto renderer = owner->getGame()->getRenderer();
    renderer->addSprite(this);
}

SpriteComponent::~SpriteComponent() {
    auto renderer = owner->getGame()->getRenderer();
    renderer->removeSprite(this);
}

void SpriteComponent::draw(Renderer *renderer) {
    if (!texture) {
        return;
    }
    int x = owner->getPosition().x;
    int y = owner->getPosition().y;
    int w = texture->getWidth();
    int h = texture->getHeight();
    SDL_Rect sr { 0, 0, w, h };

    int dx = x - w / 2;
    int dy = y - h;
    SDL_Rect dr { dx + static_cast<int>(offset.x), dy + static_cast<int>(offset.y), w, h };

    int flip = SDL_FLIP_NONE;
    if (flipX) {
        flip |= SDL_FLIP_HORIZONTAL;
    }
    if (flipY) {
        flip |= SDL_FLIP_VERTICAL;
    }
    SDL_RenderCopyEx(renderer->sdlRenderer, texture->getSdlTexture(), &sr, &dr, 0, nullptr, static_cast<SDL_RendererFlip>(flip));
}
