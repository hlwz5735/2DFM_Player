#include "engine/NativeFileDialog.hpp"
#include <axmol.h>

#if AX_TARGET_PLATFORM == AX_PLATFORM_WIN32

#include <Windows.h>
#include <commdlg.h>
#include <string>

static std::wstring utf8ToWide(const std::string& str) {
    if (str.empty()) return {};
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), result.data(), size);
    return result;
}

static std::string wideToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return {};
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), result.data(), size, nullptr, nullptr);
    return result;
}

std::string NativeFileDialog::openFileDialog(const std::string& title, const std::string& filterDesc, const std::string& filterExt) {
    // Build filter string: "KGT Files\0*.kgt\0All Files\0*.*\0\0"
    std::wstring wFilterDesc = utf8ToWide(filterDesc);
    std::wstring wFilterExt = utf8ToWide(filterExt);
    std::wstring wTitle = utf8ToWide(title);

    // Filter string is double-null terminated with nulls between parts
    std::wstring filter;
    filter += wFilterDesc;
    filter += L'\0';
    filter += wFilterExt;
    filter += L'\0';
    filter += L"All Files";
    filter += L'\0';
    filter += L"*.*";
    filter += L'\0';

    wchar_t filePath[MAX_PATH] = {0};

    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = filter.c_str();
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = wTitle.c_str();
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&ofn)) {
        std::string result = wideToUtf8(filePath);
        // Normalize path separators to forward slash
        for (auto& c : result) {
            if (c == '\\') c = '/';
        }
        return result;
    }
    return {};
}

#elif AX_TARGET_PLATFORM == AX_PLATFORM_LINUX

#include <cstdio>
#include <array>

std::string NativeFileDialog::openFileDialog(const std::string& title, const std::string& filterDesc, const std::string& filterExt) {
    // Try zenity first, then kdialog
    std::string cmd = "zenity --file-selection --title=\"" + title + "\" --file-filter=\"" + filterDesc + " | " + filterExt + "\" 2>/dev/null";

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        // Fallback to kdialog
        cmd = "kdialog --getopenfilename . \"" + filterExt + " | " + filterDesc + "\" --title \"" + title + "\" 2>/dev/null";
        pipe = popen(cmd.c_str(), "r");
        if (!pipe) return {};
    }

    std::array<char, 4096> buffer;
    std::string result;
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);

    // Remove trailing newline
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }
    return result;
}

#elif AX_TARGET_PLATFORM == AX_PLATFORM_MAC

// macOS implementation is in NativeFileDialog_mac.mm

#else

std::string NativeFileDialog::openFileDialog(const std::string& title, const std::string& filterDesc, const std::string& filterExt) {
    // Unsupported platform
    return {};
}

#endif
