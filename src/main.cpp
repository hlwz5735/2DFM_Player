#include <iostream>
#include "base/Game.hpp"

int main() {
    auto game = new Game();
    if (game->initialize()) {
        game->runLoop();
    }
    game->cleanup();
    delete game;
    return 0;
}
