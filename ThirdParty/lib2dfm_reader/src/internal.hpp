#pragma once

#include "2dfm_reader.h"
#include <vector>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

/* ═══════════════════════════════════════════════════════════════════════════
 * 流读取辅助
 * ═══════════════════════════════════════════════════════════════════════════ */

inline size_t stream_read(Kgt2dfmStream* s, void* buf, size_t size) {
    return s->read(s->ctx, buf, size);
}

inline int stream_seek(Kgt2dfmStream* s, long offset, int whence) {
    return s->seek(s->ctx, offset, whence);
}

inline long stream_tell(Kgt2dfmStream* s) {
    return s->tell(s->ctx);
}

template<typename T>
inline void stream_read_val(Kgt2dfmStream* s, T& out) {
    s->read(s->ctx, &out, sizeof(T));
}

inline void stream_skip(Kgt2dfmStream* s, long bytes) {
    s->seek(s->ctx, bytes, 1); // SEEK_CUR
}

/* ═══════════════════════════════════════════════════════════════════════════
 * 文件格式常量 (与原 2dfmCommon.hpp 一致)
 * ═══════════════════════════════════════════════════════════════════════════ */

constexpr int KGT_FILE_HEADER_SIZE = 16 + 256;
constexpr int SCRIPT_SIZE = 39;
constexpr int SCRIPT_ITEM_SIZE = 16;
constexpr int PICTURE_HEADER_SIZE = 20;
constexpr int PALETTE_SIZE = 4 * 256;
constexpr int SOUND_ITEM_HEADER_SIZE = 42;
constexpr int DEMO_CONFIG_SIZE = 8;

constexpr int MAX_PLAYER_NUM = 50;
constexpr int MAX_STAGE_NUM = 50;
constexpr int MAX_DEMO_NUM = 100;
constexpr int MAX_REACTION_NUM = 200;
constexpr int MAX_THROW_REACTION_NUM = 200;

/* ═══════════════════════════════════════════════════════════════════════════
 * 原始文件结构 (仅库内部使用)
 * ═══════════════════════════════════════════════════════════════════════════ */

#pragma pack(push, 1)

struct RawFileHeader {
    uint8_t fileSignature[16];
    char    name[256];
};

struct RawScript {
    char     scriptName[32];
    uint16_t scriptIndex;
    uint8_t  gap;
    int32_t  flags;
};

struct RawNameInfo {
    char name[256];
};

struct RawReactionItem {
    char    reactionName[32];
    int32_t isHurtAction;
};

struct RawThrowReaction {
    char name[32];
};

struct RawProjectBaseConfig {
    int32_t rawValue;
};

#pragma pack(pop)

/* ═══════════════════════════════════════════════════════════════════════════
 * 公共资源解析辅助
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * 从流中读取并填充 Kgt2dfmCommonResource。
 * 成功返回 true，失败返回 false。
 */
bool readCommonResourceFromStream(Kgt2dfmStream* stream, Kgt2dfmCommonResource* out);

/**
 * 释放 Kgt2dfmCommonResource 内部分配的内存。
 */
void freeCommonResource(Kgt2dfmCommonResource* cr);

/**
 * 解压缩实现
 */
uint8_t* decompressImpl(const uint8_t* compressed, int32_t compressedSize, int32_t destSize);

/**
 * 计算图片实际数据大小
 */
inline int32_t getPictureDataSize(const Kgt2dfmPictureHeader* h) {
    if (h->size == 0) {
        return h->width * h->height + (h->hasPrivatePalette ? 1024 : 0);
    }
    return h->size;
}
