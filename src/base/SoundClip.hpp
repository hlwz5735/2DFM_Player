#pragma once

#include <SDL_mixer.h>

namespace _2dfm {
    class Sound;
}

class SoundClip {
public:
    SoundClip() = default;
    ~SoundClip();

    static SoundClip *from2dfmSound(_2dfm::Sound *sound);

    size_t getSize() const { return size; }
    Mix_Chunk *getMixChunk() const { return mixChunk; }
private:
    Mix_Chunk *mixChunk = nullptr;
    size_t size = 0;
};