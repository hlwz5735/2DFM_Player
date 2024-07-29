#include "SoundClip.hpp"
#include "../2dfm/2dfmCommon.hpp"

SoundClip::~SoundClip() {
    //if (mixChunk) {
    //    Mix_FreeChunk(mixChunk);
    //}
    //mixChunk = nullptr;
}

SoundClip *SoundClip::from2dfmSound(_2dfm::Sound *sound) {
    return nullptr;

    // 目前只支持WAV，MIDI直接忽略
    //if (sound->header.getSoundType() != _2dfm::SoundType::WAVE) {
    //    return nullptr;
    //}

    //const auto size = sound->header.size;
    //auto rwop = SDL_RWFromMem(sound->content, size);
    //auto chunk = Mix_LoadWAV_RW(rwop, true);
    //
    //auto clip = new SoundClip();
    //clip->size = size;
    //clip->mixChunk = chunk;
    //clip->loop = sound->header.isLoop();
    //// audioSystem->addClip(clip);
    //return clip;
}
