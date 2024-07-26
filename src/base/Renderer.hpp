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

    void initSpriteVerts(const float *vertexBuffer, const unsigned int *indexBuffer);
    bool loadShaders();

    void render();

    void addSprite(SpriteComponent *sprite);
    void removeSprite(SpriteComponent *sprite);

    int getWindowWidth() const { return windowWidth; }
    int getWindowHeight() const { return windowHeight; }

protected:
private:
    int windowWidth;
    int windowHeight;

    SDL_Window *sdlWindow = nullptr;
    SDL_GLContext glContext;

    class Game *game;

    std::vector<SpriteComponent *> sprites;

    class Shader *spriteShader;
    class VertexArray *spriteVerts;
};

#endif //INC_2DFM_PLAYER_RENDERER_HPP
