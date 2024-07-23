//
// Created by 厉猛 on 2024-07-16.
//
#include <SDL.h>
#include "Game.hpp"
#include "Renderer.hpp"
#include "Texture.hpp"
#include "SpriteComponent.hpp"

Renderer::Renderer(Game *game) : game(game) {}

bool Renderer::initialize(const char *title, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    sdlWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
        SDL_WINDOW_RESIZABLE);
    if (!sdlWindow) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }
    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!sdlRenderer) {
        SDL_Log("Faild to create renderer: %s", SDL_GetError());
        return false;
    }

    return true;
}

void Renderer::cleanUp() {
    for (auto t : textures) {
        delete t;
    }
    textures.clear();
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(sdlWindow);
}

void Renderer::render() {
    // 清理颜色缓冲区
    SDL_SetRenderDrawColor(sdlRenderer, 57, 83, 164, 255);
    SDL_RenderClear(sdlRenderer);

    // 具体渲染逻辑
    for (auto sprite: sprites) {
        sprite->draw(this);
    }

    // 交换缓冲区
    SDL_RenderPresent(sdlRenderer);
}

Texture *Renderer::getTexture(size_t idx) {
    if (idx >= textures.size()) {
        return nullptr;
    }
    return textures.at(idx);
}

void Renderer::addTexture(Texture *texture) {
    if (texture) {
        auto it = std::find(textures.begin(), textures.end(), texture);
        if (it == textures.end()) {
            textures.emplace_back(texture);
        }
    }
}

void Renderer::addSprite(SpriteComponent *sprite) {
    int order = sprite->getZOrder();
    auto iter = sprites.begin();
    for (; iter != sprites.end(); ++iter) {
        if (order < (*iter)->getZOrder()) {
            break;
        }
    }
    sprites.insert(iter, sprite);
}

void Renderer::removeSprite(SpriteComponent *sprite) {
    auto iter = std::find(sprites.begin(), sprites.end(), sprite);
    if (iter != sprites.end()) {
        sprites.erase(iter);
    }
}
