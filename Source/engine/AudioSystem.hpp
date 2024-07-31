//
// Created by 厉猛 on 2024-07-16.
//

#ifndef INC_2DFM_PLAYER_AUDIOSYSTEM_HPP
#define INC_2DFM_PLAYER_AUDIOSYSTEM_HPP

#include <audio/AudioMacros.h>
#include <audio/alconfig.h>
#include <Scheduler.h>
#include <Object.h>
#include <queue>

class AudioSource;
class SoundClip;

class AudioSystem : ax::Object {
public:
    ~AudioSystem() override;

    bool initialize();

    /// 播放声音片段，返回当前声音片段的ID
    AUDIO_ID playClip(SoundClip *clip, bool loop, float volume);
    bool pause(AUDIO_ID audioId);
    bool resume(AUDIO_ID audioId);
    void stop(AUDIO_ID audioId);
    void stopAll();

    void update(float dt);

    static AudioSystem *INSTANCE;
    static AudioSystem *getInstance();
protected:
    AudioSystem();
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
