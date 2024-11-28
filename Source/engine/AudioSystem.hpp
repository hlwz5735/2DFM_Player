//
// Created by 厉猛 on 2024-07-16.
//

#ifndef INC_2DFM_PLAYER_AUDIOSYSTEM_HPP
#define INC_2DFM_PLAYER_AUDIOSYSTEM_HPP

#include <Object.h>
#include <Scheduler.h>
#include <audio/AudioMacros.h>
#include <audio/alconfig.h>
#include <queue>

#include "Singleton.hpp"

class AudioSource;
class SoundClip;

class AudioSystem : ax::Object, public Singleton<AudioSystem> {
public:
    AudioSystem();

    AudioSystem(const AudioSystem &o) = delete;
    AudioSystem & operator=(const AudioSystem &o) = delete;

    bool initialize();
    void cleanup();

    /// 播放声音片段，返回当前声音片段的ID
    AUDIO_ID playClip(SoundClip *clip, bool loop, float volume);
    bool pause(AUDIO_ID audioId);
    bool resume(AUDIO_ID audioId);
    void stop(AUDIO_ID audioId);
    void stopAll();

    void update(float dt);
private:
    ALuint findValidSource();
    void updateForStop();

    ALuint alSources[MAX_AUDIOINSTANCES];
    std::queue<ALuint> unusedSourceQueue;
    AUDIO_ID currentAudioId = 0;
    std::unordered_map<AUDIO_ID, AudioSource *> sourceMap;

    ax::Scheduler *scheduler = nullptr;
    ALCdevice* aLDevice;
    ALCcontext* aLContext;
};

#endif //INC_2DFM_PLAYER_AUDIOSYSTEM_HPP
