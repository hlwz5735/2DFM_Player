//
// Created by limen on 2024/8/1.
//

#include "AudioSource.hpp"
#include "SoundClip.hpp"
#include <Logging.h>

AudioSource::AudioSource() {}

AudioSource::~AudioSource() {
    destroy();
}

void AudioSource::destroy() {
    if (destroyed) {
        return;
    }
    destroyed = true;
    alSourceStop(alSourceId);
    alSourcei(alSourceId, AL_BUFFER, 0);
    ready = false;
    willDelete = true;
}

bool AudioSource::setLoop(bool loop) {
    if (!destroyed) {
        this->loop = loop;
        return true;
    }
    return false;
}

bool AudioSource::isFinised() const {
    ALint sourceState;
    alGetSourcei(alSourceId, AL_SOURCE_STATE, &sourceState);
    return sourceState == AL_STOPPED;
}

bool AudioSource::play2d() {
    if (destroyed) {
        return false;
    }
    bool ret = false;
    do {
        alSourcei(alSourceId, AL_BUFFER, soundClip->getAlBufferId());
        alSourcef(alSourceId, AL_PITCH, 1.f);
        alSourcef(alSourceId, AL_GAIN, volume);
        alSourcei(alSourceId, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
        alSourcePlay(alSourceId);

        auto alError = alGetError();
        if (alError != AL_NO_ERROR) {
            AXLOGE("{}:alSourcePlay error code:{:#x}", __FUNCTION__, (int)alError);
            break;
        }
        ALint state;
        alGetSourcei(alSourceId, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING) {
            AXLOGE("state isn't playing, {}", state);
        }
        assert(state == AL_PLAYING);
        ready = true;
        ret = true;
    } while (false);
    if (!ret) {
        willDelete = true;
    }
    return ret;
}
