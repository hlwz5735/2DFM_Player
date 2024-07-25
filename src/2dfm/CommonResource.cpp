#include "CommonResource.hpp"
#include "../base/Texture.hpp"
#include "../base/SoundClip.hpp"

CommonResource::~CommonResource() {
    for (auto t : this->pictures) {
        delete t;
    }
    this->pictures.clear();
    for (auto s : this->scripts) {
        delete s;
    }
    this->scripts.clear();
    for (auto s : this->scriptItems) {
        delete s;
    }
    this->scriptItems.clear();
    for (auto &p : this->sharedPalettes) {
        SDL_FreePalette(p);
        p = nullptr;
    }
    for (auto s : this->sounds) {
        delete s;
    }
    this->sounds.clear();
}
