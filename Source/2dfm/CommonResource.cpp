#include "CommonResource.hpp"
#include "engine/SoundClip.hpp"

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

std::vector<KgtScript>::iterator CommonResource::findNextWithFlag(
    std::vector<KgtScript>::iterator it,
    std::vector<KgtScript>::iterator end,
    ScriptSpecialFlag flag
) {
    while (it != end) {
        if (it->flags == flag) {
            return it;
        }
        ++it;
    }
    return end;
}
