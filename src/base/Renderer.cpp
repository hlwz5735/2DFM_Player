//
// Created by 厉猛 on 2024-07-16.
//
#include "Renderer.hpp"
#include "Game.hpp"
#include "Texture.hpp"
#include "SpriteComponent.hpp"
#include "VertexArray.hpp"
#include "Shader.hpp"
#include <SDL.h>
#include <SDL_image.h>
#include <GL/glew.h>

Renderer::Renderer(Game *game) : game(game) {}

bool Renderer::initialize(const char *title, int width, int height) {
    windowWidth = width;
    windowHeight = height;

    if (SDL_Init(SDL_INIT_VIDEO
        | SDL_INIT_AUDIO
        | SDL_INIT_GAMECONTROLLER) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }
    if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) == 0) {
        SDL_Log("Unable to initialize SDL_image: %s", SDL_GetError());
        return false;
    }

    // 设置使用核心配置
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    // 指定 3.3 版本
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    // 指定缓冲区的颜色通道为8位
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    // 启用双缓冲区
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    // 强制使用硬件加速
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    sdlWindow = SDL_CreateWindow(title,
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!sdlWindow) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    glContext = SDL_GL_CreateContext(sdlWindow);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        SDL_Log("Failed to initialize GLEW.");
        return false;
    }
    // GLEW在某些平台下会先报出一个异常码，清理掉它
    glGetError();

    float verticles[] = {
            -.5f, .5f, 0, 0, 0,
            .5f, .5f, 0, 1.f, 0,
            .5f, -.5f, 0, 1.f, 1.f,
            -.5f, -.5f, 0, 0, 1.f
    };
    unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
    };
    initSpriteVerts(verticles, indices);
    loadShaders();

    return true;
}

void Renderer::cleanUp() {
    delete spriteVerts;
    spriteShader->unload();
    delete spriteShader;
    if (glContext) {
        SDL_GL_DeleteContext(glContext);
    }
    SDL_DestroyWindow(sdlWindow);
}

void Renderer::render() {
    // 清理颜色缓冲区
    glClearColor(0, 0, 0, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 启用颜色混合，设置使用线性混合模式
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 具体渲染逻辑
    spriteShader->setActive();
    spriteVerts->setActive();
    for (auto sprite : sprites) {
        sprite->draw(spriteShader);
    }

    // 交换缓冲区
    SDL_GL_SwapWindow(sdlWindow);
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
void Renderer::initSpriteVerts(const float *vertexBuffer, const unsigned int *indexBuffer) {
    spriteVerts = new VertexArray(vertexBuffer, 4, indexBuffer, 6);
}
bool Renderer::loadShaders() {
    spriteShader = new Shader();
    if (!spriteShader->load("Shaders/Sprite.vert", "Shaders/Sprite.frag")) {
        return false;
    }
    spriteShader->setActive();
    auto mVP = Matrix4::createSimpleViewProj(static_cast<float>(windowWidth), static_cast<float>(windowHeight));
    spriteShader->setMatrixUniform("uVP", mVP);
    return true;
}
