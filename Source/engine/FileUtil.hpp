#pragma once

#include <cstdio>
#include <string>

#if defined(_WIN32)
#    include <windows.h>
#endif

/**
 * 以 UTF-8 路径打开文件。
 *
 * Windows 上 std::fopen 会按 ANSI 代码页解释路径，导致 UTF-8 编码的
 * 中文路径（例如中文命名的 KGT 游戏）无法打开，因此这里改用 _wfopen。
 */
inline FILE* fopenUtf8(const std::string& path, const char* mode) {
#if defined(_WIN32)
    auto toWide = [](const std::string& str) {
        if (str.empty()) return std::wstring();
        int size = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0);
        if (size <= 0) return std::wstring();
        std::wstring result(static_cast<size_t>(size), L'\0');
        ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), result.data(), size);
        return result;
    };
    return ::_wfopen(toWide(path).c_str(), toWide(mode).c_str());
#else
    return std::fopen(path.c_str(), mode);
#endif
}
