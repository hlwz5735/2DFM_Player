//
// Created by 厉猛 on 2024-07-16.
//

#include "AudioSystem.hpp"
#include "SoundClip.hpp"
#include "AudioSource.hpp"
#include <AudioEngine.h>
#include <PlatformMacros.h>
#include <Director.h>

AudioSystem *AudioSystem::INSTANCE = nullptr;

AudioSystem *AudioSystem::getInstance() {
    if (!INSTANCE) {
        INSTANCE = new AudioSystem();
        INSTANCE->initialize();
    }
    return INSTANCE;
}

AudioSystem::AudioSystem() {}

AudioSystem::~AudioSystem() {
    if (scheduler != nullptr) {
        scheduler->unschedule(AX_SCHEDULE_SELECTOR(AudioSystem::update), this);
    }
    if (aLContext) {
        alDeleteSources(MAX_AUDIOINSTANCES, alSources);

        alcMakeContextCurrent(nullptr);
        alcDestroyContext(aLContext);
        aLContext = nullptr;
    }

    if (aLDevice) {
        alcCloseDevice(aLDevice);
        aLDevice = nullptr;
    }
}

bool AudioSystem::initialize() {
    bool ret = false;
    do {
        aLDevice = alcOpenDevice(nullptr);
        if (aLDevice) {
            aLContext = alcCreateContext(aLDevice, nullptr);
            alcMakeContextCurrent(aLContext);
            alGenSources(MAX_AUDIOINSTANCES, alSources);
            auto alError = alGetError();
            if (alError != AL_NO_ERROR) {
                AXLOGE("{}:generating sources failed! error = {:#x}\n", __FUNCTION__, alError);
                break;
            }
            for (int i = 0; i < MAX_AUDIOINSTANCES; ++i) {
                unusedSourceQueue.push(alSources[i]);
            }
            scheduler = ax::Director::getInstance()->getScheduler();
            scheduler->schedule(AX_SCHEDULE_SELECTOR(AudioSystem::update), this, 0.05f, false);
            ret = true;
        }
    } while (false);
    return ret;
}

AUDIO_ID AudioSystem::playClip(SoundClip *clip, bool loop, float volume) {
    if (!clip) {
        return ax::AudioEngine::INVALID_AUDIO_ID;
    }

    ALuint alSource = findValidSource();
    if (alSource == AL_INVALID) {
        return ax::AudioEngine::INVALID_AUDIO_ID;
    }
    auto player = new AudioSource;
    if (player == nullptr) {
        return ax::AudioEngine::INVALID_AUDIO_ID;
    }
    player->alSourceId = alSource;
    player->loop = loop;
    player->volume = volume;
    sourceMap.emplace(++currentAudioId, player);
    player->setClip(clip);
    player->play2d();

    return currentAudioId;
}

ALuint AudioSystem::findValidSource() {
    ALuint sourceId = AL_INVALID;
    if (!unusedSourceQueue.empty()) {
        sourceId = unusedSourceQueue.front();
        unusedSourceQueue.pop();
    }
    return sourceId;
}

bool AudioSystem::pause(AUDIO_ID audioID) {
    auto iter = sourceMap.find(audioID);
    if (iter == sourceMap.end()) {
        return false;
    }

    auto player = iter->second;
    bool ret = true;
    alSourcePause(player->alSourceId);
    auto error = alGetError();
    if (error != AL_NO_ERROR) {
        ret = false;
        AXLOGE("{}: audio id = {}, error = {:#x}\n", __FUNCTION__, audioID, error);
    }
    return ret;
}

bool AudioSystem::resume(AUDIO_ID audioId) {
    auto iter = sourceMap.find(audioId);
    if (iter == sourceMap.end()) {
        return false;
    }

    auto player = iter->second;
    bool ret = true;
    alSourcePlay(player->alSourceId);
    auto error = alGetError();
    if (error != AL_NO_ERROR) {
        ret = false;
        AXLOGE("{}: audio id = {}, error = {:#x}\n", __FUNCTION__, audioId, error);
    }

    return ret;
}

void AudioSystem::stop(AUDIO_ID audioId) {
    auto iter = sourceMap.find(audioId);
    if (iter == sourceMap.end()) {
        return;
    }

    auto player = iter->second;
    player->destroy();

    updateForStop();
}
void AudioSystem::stopAll() {
    for (auto &[id, player] : sourceMap) {
        player->destroy();
    }

    updateForStop();
}
void AudioSystem::update(float dt) {
    updateForStop();
}

void AudioSystem::updateForStop() {
    AUDIO_ID audioId;
    AudioSource *player;
    ALuint alSource;
    for (auto it = sourceMap.begin(); it != sourceMap.end(); ) {
        audioId = it->first;
        player = it->second;
        alSource = player->alSourceId;
        if (player->willDelete) {
            it = sourceMap.erase(it);
            delete player;
            unusedSourceQueue.push(alSource);
        } else if (player->ready && player->isFinised()) {
            it = sourceMap.erase(it);
            delete player;
            unusedSourceQueue.push(alSource);
        } else {
            ++it;
        }
    }
}
