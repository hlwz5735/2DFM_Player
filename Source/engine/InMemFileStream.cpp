//
// Created by limen on 2026/2/8.
//

#include "InMemFileStream.hpp"
#include <Macros.h>

InMemFileStream::InMemFileStream(InMemFileStream &&o) noexcept
    : ax::IFileStream(std::move(o))
    , buffer(std::move(o.buffer)) {}

InMemFileStream &InMemFileStream::operator=(InMemFileStream &&o) noexcept {
    if (this == &o) {
        return *this;
    }
    ax::IFileStream::operator =(std::move(o));
    buffer = std::move(o.buffer);
    return *this;
}

bool InMemFileStream::open(std::string_view path, IFileStream::Mode mode) {
    return true;
}

int64_t InMemFileStream::seek(int64_t offset, int origin) const {
    int64_t fileSize = static_cast<int64_t>(buffer.size());
    int64_t new_pos = -1;

    switch (origin) {
    case SEEK_SET:
        new_pos = offset;
        break;
    case SEEK_CUR:
        new_pos = static_cast<int64_t>(position) + offset;
        break;
    case SEEK_END:
        new_pos = fileSize + offset;
        break;
    default:
        return -1;
    }

    // 验证新位置是否有效: [0, file_size]
    if (new_pos < 0 || new_pos > fileSize) {
        return -1;
    }

    position = static_cast<size_t>(new_pos);
    return new_pos;
}

int InMemFileStream::read(void *buf, unsigned size) const {
    // 注意：读取长度必须遵循 axmol 的 IFileStream 约定 —— 返回实际读到的字节数，
    // 数据读完（EOF）时返回 0，只有真正的 I/O 错误才返回 -1。
    // 这里曾经在 EOF 返回 -1，而 AudioDecoderWav::read() 会把返回值当成无符号帧数参与运算：
    //   bytesToFrames((uint32_t)-1) = 2147483647 帧 → framesToBytes() = 0xFFFFFFFE
    //   → alBufferData(..., -2, ...) → OpenAL Soft 每帧刷 "Negative storage size -2"
    // （音乐播完后播放器仍在续缓冲，于是无限刷屏）
    if (!buf || size == 0) {
        return 0;
    }
    if (position >= buffer.size()) {
        return 0;
    }

    unsigned available = buffer.size() - position;
    const size_t toRead = std::min(size, available);

    std::memcpy(buf, buffer.data() + position, toRead);
    position += toRead;

    return static_cast<int>(toRead);
}

int InMemFileStream::closeInternal() {
    buffer.clear();
    return 0;
}
