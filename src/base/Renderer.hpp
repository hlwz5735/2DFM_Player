//
// Created by 厉猛 on 2024-07-16.
//

#ifndef INC_2DFM_PLAYER_RENDERER_HPP
#define INC_2DFM_PLAYER_RENDERER_HPP

class Renderer {
public:
    bool initialize(const char *title, int width, int height);
    void cleanUp();

    void render();

private:
    int windowWidth;
    int windowHeight;

    struct SDL_Window *sdlWindow = nullptr;
    struct SDL_Renderer *sdlRenderer;
};

#endif //INC_2DFM_PLAYER_RENDERER_HPP
