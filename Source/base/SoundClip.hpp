#pragma once

namespace _2dfm {
    class Sound;
}

class SoundClip {
public:
    SoundClip() = default;
    ~SoundClip();

    static SoundClip *from2dfmSound(_2dfm::Sound *sound);

    size_t getSize() const { return size; }
    // Mix_Chunk *getMixChunk() const { return mixChunk; }
    bool isLoop() const { return loop; }
private:
    // Mix_Chunk *mixChunk = nullptr;
    size_t size = 0;
    bool loop = false;
};
