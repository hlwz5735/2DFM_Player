#include "CommonResource.hpp"
#include "../base/SoundClip.hpp"
#include <axmol.h>

CommonResource::~CommonResource() {
    for (auto t : this->pictures) {
        delete t;
    }
    this->pictures.clear();
    this->scripts.clear();
    for (auto s : this->scriptItems) {
        delete s;
    }
    this->scriptItems.clear();
    for (auto &p : this->sharedPalettes) {
        delete p;
        p = nullptr;
    }
    for (auto s : this->sounds) {
        delete s;
    }
    this->sounds.clear();
}