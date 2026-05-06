#include "internal.hpp"
#include <cstdio>

// ─── FILE* 适配器 ─────────────────────────────────────────────────────────────

static size_t file_read(void* ctx, void* buf, size_t size) {
    return std::fread(buf, 1, size, static_cast<FILE*>(ctx));
}

static int file_seek(void* ctx, long offset, int whence) {
    return std::fseek(static_cast<FILE*>(ctx), offset, whence);
}

static long file_tell(void* ctx) {
    return std::ftell(static_cast<FILE*>(ctx));
}

Kgt2dfmStream kgt_stream_from_file(void* file_ptr) {
    Kgt2dfmStream s{};
    s.read = file_read;
    s.seek = file_seek;
    s.tell = file_tell;
    s.ctx  = file_ptr;
    return s;
}

// ─── 工具 API ─────────────────────────────────────────────────────────────────

uint8_t* kgt_decompress(const uint8_t* compressed, int32_t compressedSize, int32_t destSize) {
    return decompressImpl(compressed, compressedSize, destSize);
}

void kgt_free_buffer(void* ptr) {
    std::free(ptr);
}
