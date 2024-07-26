//
// Created by 厉猛 on 2024-07-16.
//

#ifndef INC_2DFM_PLAYER_RENDERER_HPP
#define INC_2DFM_PLAYER_RENDERER_HPP

#include <SDL.h>
#include <vector>
#include "Game.hpp"

class SpriteFrame;
class Texture;
class SpriteComponent;

class Renderer {
public:
    friend class Game;
    friend class SpriteComponent;

    explicit Renderer(Game *game);

    bool initialize(const char *title, int width, int height);
    void cleanUp();

    void render();

    SDL_Renderer *getSdlRenderer() { return sdlRenderer; }

    //Texture *createTexture(size_t idx);
    //void addTexture(Texture *texture);
    void addSprite(SpriteComponent *sprite);
    void removeSprite(SpriteComponent *sprite);
protected:
private:
    int windowWidth;
    int windowHeight;

    SDL_Window *sdlWindow = nullptr;
    SDL_Renderer *sdlRenderer = nullptr;

    class Game *game;

    //std::vector<Texture *> textures;
    std::vector<SpriteComponent *> sprites;
};

#endif //INC_2DFM_PLAYER_RENDERER_HPP
