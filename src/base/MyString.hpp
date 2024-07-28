#pragma once

#include <iconv.h>
#include <string>
#include <stdexcept>

inline std::string gb2312ToUtf8(const char *gb2312) {
    auto len = strlen(gb2312);
    if (len == 0) {
        return "";
    }
    // 分配足够大的空间，防止溢出
    std::vector<char> utf8(len * 4); 
    char *inbuf = const_cast<char *>(gb2312);
    size_t inbytesleft = len;
    char *outbuf = &utf8[0];
    size_t outbytesleft = utf8.size();

    iconv_t cd = iconv_open("UTF-8", "GB2312");
    if (cd == (iconv_t)-1) {
        throw std::runtime_error("iconv_open failed");
    }

    if (iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft) == static_cast<size_t>(-1)) {
        iconv_close(cd);
        throw std::runtime_error("iconv failed");
    }

    iconv_close(cd);
    // 调整大小以去除未使用的空间
    utf8.resize(utf8.size() - outbytesleft); 
    return { utf8.begin(), utf8.end() };
}
