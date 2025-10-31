#pragma once

// #include <alconfig.h>
#include <cstdint>
#include <al.h>

namespace _2dfm {
class Sound;
}

class SoundClip {
public:
    SoundClip(ALuint bufferId): alBufferId(bufferId) {}
    ~SoundClip();

    static SoundClip *from2dfmSound(_2dfm::Sound *sound);

    int64_t getSize() const { return size; }
    bool isLoop() const { return loop; }
    ALuint getAlBufferId() const { return alBufferId; }
private:
    ALuint alBufferId;
    int64_t size = 0;
    bool loop = false;
};
