//
// Created by 厉猛 on 2024-07-16.
//
#include "AudioSystem.hpp"
#include "SoundClip.hpp"
#include <SDL.h>
#include <SDL_mixer.h>

AudioSystem::AudioSystem(Game *game): game(game)
{}

bool AudioSystem::initialize() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("Sdl_mixer could not initialize! SDL_mixer Error: %s", Mix_GetError());
        SDL_Quit();
        return false;
    }
    return true;
}

void AudioSystem::cleanUp() {
    for (auto c : clipPool) {
        delete c;
    }
    clipPool.clear();
    Mix_CloseAudio();
}

void AudioSystem::addClip(SoundClip *clip) {
    auto it = std::find(clipPool.begin(), clipPool.end(), clip);
    if (it == clipPool.end()) {
        clipPool.emplace_back(clip);
    }
}

void AudioSystem::removeClip(SoundClip *clip) {
    auto it = std::find(clipPool.begin(), clipPool.end(), clip);
    if (it != clipPool.end()) {
        clipPool.erase(it);
    }
    delete clip;
}

void AudioSystem::playClip(SoundClip *clip, bool loop) {
    if (clip && clip->getSize() > 0 && clip->getMixChunk()) {
        Mix_PlayChannel(-1, clip->getMixChunk(), loop ? -1 : 0);
    }
}

void AudioSystem::stopAll() {
    Mix_HaltChannel(-1);
}
