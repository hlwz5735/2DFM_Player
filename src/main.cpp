#include <iostream>
#include "base/Game.hpp"

#ifdef WIN32
#include <Windows.h>
#include <locale.h>
#ifdef main
#undef main
#endif // main
#endif

int main() {
#ifdef WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, ".utf-8");
#endif
    auto game = new Game();
    if (game->initialize()) {
        game->runLoop();
    }
    game->cleanUp();
    delete game;
    return 0;
}
