#pragma once

#include <cstdint>
#include <string>

#include "2dfm/2dfmCommon.hpp"

namespace _2dfm {
class Sound;
}

class SoundClip {
public:
    SoundClip(_2dfm::Sound *sound, std::string vfp): rawSoundPtr(sound), virtualFilePath(vfp) {}
    ~SoundClip();

    int64_t getSize() const;
    bool isLoop() const;

    const std::string &getVirtualFilePath() const { return virtualFilePath; }

    _2dfm::Sound *getRawSoundPtr() const { return rawSoundPtr; }
private:
    std::string virtualFilePath;
    _2dfm::Sound *rawSoundPtr;
};
