//
// Created by limen on 2024/8/1.
//

#ifndef AUDIOSOURCE_HPP
#define AUDIOSOURCE_HPP

// #include <audio/alconfig.h>
#include <al.h>

class SoundClip;
class AudioSource {
public:
    friend class AudioSystem;

    AudioSource();
    ~AudioSource();

    void destroy();

    bool setLoop(bool loop);
    bool isFinised() const;

protected:
    void setClip(SoundClip *clip) { soundClip = clip; }
    bool play2d();

    bool loop = false;
    bool destroyed = false;
    bool ready = false;
    SoundClip *soundClip = nullptr;
    ALuint alSourceId = 0;
    float volume = 1.f;
    /// 即将移除标记
    bool willDelete = false;
};

#endif //AUDIOSOURCE_HPP
