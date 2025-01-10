#pragma once

#include <iconv.h>
#include <stdexcept>
#include <string>

inline std::string gbkToUtf8(const char *gb2312) {
    auto len = strlen(gb2312);
    if (len == 0) {
        return "";
    }
    // 分配足够大的空间，防止溢出
    std::vector<char> utf8(len * 4);
    char *inBuf = const_cast<char *>(gb2312);
    size_t inBytesLeft = len;
    char *outBuf = &utf8[0];
    size_t outBytesLeft = utf8.size();

    iconv_t cd = iconv_open("UTF-8", "GBK");
    if (cd == (iconv_t)-1) {
        throw std::runtime_error("iconv_open failed");
    }

    auto res = iconv(cd, &inBuf, &inBytesLeft, &outBuf, &outBytesLeft);
    if (res == static_cast<size_t>(-1)) {
        iconv_close(cd);
        // throw std::runtime_error("iconv failed");
        return std::string("？？？？？");
    }

    iconv_close(cd);
    // 调整大小以去除未使用的空间
    utf8.resize(utf8.size() - outBytesLeft);
    return {utf8.begin(), utf8.end() };
}
