//
// Created by limen on 2026/2/8.
//

#pragma once
#include <vector>
#include "IFileStream.h"
#include "2dfm/2dfmCommon.hpp"

class InMemFileStream : public ax::IFileStream {
public:
    InMemFileStream() = default;
    ~InMemFileStream() override = default;

    InMemFileStream(const InMemFileStream&) = delete;
    InMemFileStream(InMemFileStream&&) noexcept;
    InMemFileStream& operator=(const InMemFileStream&) = delete;
    InMemFileStream& operator=(InMemFileStream&&) noexcept;

    bool open(std::string_view path, IFileStream::Mode mode) override;
    int close() override { return closeInternal(); }
    int64_t seek(int64_t offset, int origin) const override;
    int read(void *buf, unsigned size) const override;
    int write(const void *buf, unsigned size) const override { return -1; };
    int64_t tell() const override { return position;};
    int64_t size() const override { return buffer.size(); }
    bool isOpen() const override { return true;}


    void setBuffer(const std::vector<byte>& data) {
        buffer = data;
        position = 0;
    }

    void setBuffer(std::vector<byte>&& data) {
        buffer = std::move(data);
        position = 0;
    }
private:
    int closeInternal();

    std::vector<byte> buffer;
    mutable std::size_t position = 0;
};
