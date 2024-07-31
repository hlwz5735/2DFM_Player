//
// Created by limen on 2024/7/31.
//

#ifndef WAVDECODER_HPP
#define WAVDECODER_HPP

#include <cstdint>

struct DecodedSoundData {
    void *data;
    int64_t size;
    int channels;
    int samplerate;
};

DecodedSoundData decodeWavFile(void *rawData, int64_t size);

#endif //WAVDECODER_HPP
