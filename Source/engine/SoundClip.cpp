#include "SoundClip.hpp"
#include "2dfm/2dfmCommon.hpp"

#include <AudioEngine.h>
#include <Logging.h>

SoundClip::~SoundClip() {
    ax::AudioEngine::uncache(this->virtualFilePath);
    if (rawSoundPtr != nullptr) {
        delete rawSoundPtr;
    }
}
int64_t SoundClip::getSize() const {
    if (rawSoundPtr) {
        return rawSoundPtr->header.size;
    }
    return 0;
}

bool SoundClip::isLoop() const {
    if (rawSoundPtr) {
        return rawSoundPtr->header.isLoop();
    }
    return false;
}
