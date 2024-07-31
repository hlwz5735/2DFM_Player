#include "SoundClip.hpp"
#include "2dfm/2dfmCommon.hpp"
#include "WavDecoder.hpp"
#include <Logging.h>

SoundClip::~SoundClip() {
    if (alBufferId != 0) {
        alDeleteBuffers(1, &alBufferId);
    }
}

SoundClip *SoundClip::from2dfmSound(_2dfm::Sound *sound) {
    // 目前只支持WAV，MIDI直接忽略
    if (sound->header.getSoundType() != _2dfm::SoundType::WAVE) {
        return nullptr;
    }

    const auto size = sound->header.size;
    auto decodedData = decodeWavFile(sound->content, size);

    // 在这里可以进行后续的声音处理或读取操作

    ALenum format;
    if (decodedData.channels == 1) {
        format = AL_FORMAT_MONO16;
    } else if (decodedData.channels == 2) {
        format = AL_FORMAT_STEREO16;
    } else {
        AXLOGE("Unsupported number of channels");
        free(decodedData.data);
        decodedData.data = nullptr;
        throw std::runtime_error("failed.");
    }

    ALuint bufferId;
    alGenBuffers(1, &bufferId);
    // 将音频数据上传到缓冲区
    const auto bufferSize = static_cast<ALsizei>(decodedData.size);
    alBufferData(bufferId, format, decodedData.data, bufferSize * sizeof(short), decodedData.samplerate);
    free(decodedData.data);
    decodedData.data = nullptr;

    auto clip = new SoundClip(bufferId);
    clip->size = size;
    clip->loop = sound->header.isLoop();
    return clip;
}
