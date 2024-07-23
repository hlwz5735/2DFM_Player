#include "TestComponent.hpp"
#include "../2dfm/KgtGame.hpp"
#include "../base/Game.hpp"
#include "../base/InputSystem.hpp"
#include "../base/Node.hpp"
#include "../base/Renderer.hpp"
#include "../base/SpriteComponent.hpp"

TestComponent::TestComponent(Node *owner, int updateOrder): Component(owner, updateOrder) {
    sprite = owner->getComponent<SpriteComponent>();
}

void TestComponent::update(float deltaTime) {
    if (kgtGame && sprite) {
        auto input = Game::getInstance()->getInputSystem();

        if (input->isKeyDown(SDL_SCANCODE_RIGHT)) {
            textureId += 1;
            if (textureId >= kgtGame->spriteFrames.size()) {
                textureId = 0;
            }
        }
        if (input->isKeyDown(SDL_SCANCODE_LEFT)) {
            if (textureId <= 0) {
                textureId = kgtGame->spriteFrames.size();
            }
            textureId -= 1;
        }

        auto texture = Game::getInstance()->getRenderer()->getTexture(textureId);
        sprite->setTexture(texture);
    }
}
