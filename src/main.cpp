#include <iostream>
#include "base/Game.hpp"

#ifdef WIN32
#ifdef main
#undef main
#endif // main
#endif

int main() {
    auto game = new Game();
    if (game->initialize()) {
        game->runLoop();
    }
    game->cleanUp();
    delete game;
    return 0;
}
