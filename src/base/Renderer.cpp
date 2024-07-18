//
// Created by 厉猛 on 2024-07-16.
//
#include <SDL.h>
#include "Renderer.hpp"

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
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(sdlWindow);
}

void Renderer::render() {
    // 清理颜色缓冲区
    SDL_SetRenderDrawColor(sdlRenderer, 57, 83, 164, 255);
    SDL_RenderClear(sdlRenderer);

//    // 具体渲染逻辑
//    for (auto sprite: sprites) {
//        sprite->draw(renderer);
//    }

    // 交换缓冲区
    SDL_RenderPresent(sdlRenderer);
}
