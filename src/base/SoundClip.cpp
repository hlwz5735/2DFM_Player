#include "SoundClip.hpp"
#include "AudioSystem.hpp"
#include "Game.hpp"
#include "../2dfm/2dfmCommon.hpp"

SoundClip::~SoundClip() {
    if (mixChunk) {
        Mix_FreeChunk(mixChunk);
    }
    mixChunk = nullptr;
}

SoundClip *SoundClip::from2dfmSound(_2dfm::Sound *sound) {
    // 目前只支持WAV，MIDI直接忽略
    if (sound->header.soundType == _2dfm::SoundType::MIDI) {
        return nullptr;
    }
    auto audioSystem = Game::getInstance()->getAudioSystem();

    const auto size = sound->header.size;
    auto rwop = SDL_RWFromMem(sound->content, size);
    auto chunk = Mix_LoadWAV_RW(rwop, true);
    
    auto clip = new SoundClip();
    clip->size = size;
    clip->mixChunk = chunk;
    audioSystem->addClip(clip);
    return clip;
}
