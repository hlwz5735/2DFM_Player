#include "TestComponent.hpp"
#include "../2dfm/CommonResource.hpp"
#include "../2dfm/2dfmScriptItem.hpp"
#include "../base/Game.hpp"
#include "../base/InputSystem.hpp"
#include "../base/Node.hpp"
#include "../base/SpriteComponent.hpp"

TestComponent::TestComponent(Node *owner, int updateOrder): Component(owner, updateOrder) {
    sprite = owner->getComponent<SpriteComponent>();
}

void TestComponent::update(float deltaTime) {
    if (cr && sprite) {
        auto input = Game::getInstance()->getInputSystem();

        if (input->isKeyDown(SDL_SCANCODE_RIGHT)) {
            picNo += 1;
            if (picNo >= cr->spriteFrames.size()) {
                picNo = 0;
            }
        }
        if (input->isKeyDown(SDL_SCANCODE_LEFT)) {
            if (picNo <= 0) {
                picNo = cr->spriteFrames.size();
            }
            picNo -= 1;
        }

        const auto t = cr->pictures.at(picNo);
        if (sprite->getTexture() != t) {
            const auto &s = cr->spriteFrames.at(picNo);
            SDL_Log("picNo: %d/%d, width: %d, height: %d, privateP: %c",
                picNo, cr->pictures.size(), s.width, s.height, s.hasPrivatePalette ? 'o' : 'x');

            sprite->setTexture(cr->pictures.at(picNo));
        }
    }
}
