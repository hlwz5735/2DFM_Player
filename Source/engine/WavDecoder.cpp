//
// Created by limen on 2024/7/31.
//

#include "WavDecoder.hpp"
#include "2dfm/2dfmCommon.hpp"
#include <Logging.h>
#include <sndfile.h>

// 定义一个自定义的内存读取结构体
typedef struct {
    byte *data;   // 内存数据指针
    sf_count_t pos;     // 当前读取位置
    sf_count_t size;    // 数据大小
} MemFile;

// 自定义的读取函数，用于从内存中读取数据
static sf_count_t mem_fread(void *ptr, sf_count_t count, void *user_data) {
    auto memfile = static_cast<MemFile *>(user_data);

    // 剩余待读取字节数
    sf_count_t remain = memfile->size - memfile->pos;
    // 本次读取的数量
    sf_count_t to_read = (count < remain) ? count : remain;

    if (to_read > 0) {
        memcpy(ptr, memfile->data + memfile->pos, to_read);
        memfile->pos += to_read;
        return to_read;
    } else {
        return 0;
    }
}

static sf_count_t mem_ftell(void *user_data) {
    auto memfile = static_cast<MemFile *>(user_data);
    return memfile->pos;
}

static sf_count_t mem_flen(void *user_data) {
    auto memfile = static_cast<MemFile *>(user_data);
    return memfile->size;
}

static sf_count_t mem_fseek(sf_count_t offset, int whence, void *user_data) {
    auto memfile = static_cast<MemFile *>(user_data);
    if (whence == SEEK_SET) {
        if (offset > memfile->size) {
            memfile->pos = memfile->size;
        } else {
            memfile->pos = offset;
        }
    } else if (whence == SEEK_CUR) {
        sf_count_t remain = memfile->size - memfile->pos;
        sf_count_t to_read = (offset < remain) ? offset : remain;
        memfile->pos += to_read;
    } else if (whence == SEEK_END) {
        sf_count_t pos = memfile->size + offset;
        if (pos > memfile->size) {
            pos = memfile->size;
        }
        if (pos < 0) {
            pos = 0;
        }
        memfile->pos = pos;
    }
    return memfile->pos;
}

// 设置虚拟IO结构体
static SF_VIRTUAL_IO virtual_io = {
    .get_filelen = mem_flen,
    .seek = mem_fseek,
    .read = mem_fread,
    .write = nullptr,              // 不需要实现写入功能
    .tell = mem_ftell
};

DecodedSoundData decodeWavFile(void *rawData, int64_t size) {
    MemFile memfile = {
        .data = static_cast<byte *>(rawData),
        .pos = 0,
        .size = size
    };

    SF_INFO sfinfo;
    SNDFILE *sndfile = sf_open_virtual(&virtual_io, SFM_READ, &sfinfo, (void *)&memfile);
    if (!sndfile) {
        AXLOGE("Error opening sound file.");
        throw std::runtime_error("failed.");
    }

    auto bufferSize = sfinfo.frames * sfinfo.channels;
    // TODO: 这里武断地断定WAV文件使用了16位波形
    auto bufferData = malloc(bufferSize * sizeof(short));
    sf_read_short(sndfile, static_cast<short *>(bufferData), bufferSize);

    // Clean up libsndfile resources
    sf_close(sndfile);

    return { bufferData, bufferSize, sfinfo.channels, sfinfo.samplerate };
    // delete[] bufferData;
}
