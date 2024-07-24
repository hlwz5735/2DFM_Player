#include "KgtGame.hpp"
#include "../base/Texture.hpp"

void cleanKgtGame(KgtGame *game) {
    for (auto t : game->pictures) {
        delete t;
    }
    game->pictures.clear();
}
