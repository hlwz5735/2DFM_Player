//
// Created by 厉猛 on 2024-07-17.
//
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iconv.h>
#include "../src/2dfm/2dfmFile.hpp"
using namespace std;

std::string gb2312ToUtf8(const char *gb2312) {
    auto len = strlen(gb2312);
    std::vector<char> utf8(len * 4); // 分配足够大的空间，防止溢出
    char* inbuf = const_cast<char *>(gb2312);
    size_t inbytesleft = len;
    char* outbuf = &utf8[0];
    size_t outbytesleft = utf8.size();

    iconv_t cd = iconv_open("UTF-8", "GB2312");
    if (cd == (iconv_t)-1) {
        throw std::runtime_error("iconv_open failed");
    }

    if (iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft) == (size_t) -1) {
        iconv_close(cd);
        throw std::runtime_error("iconv failed");
    }

    iconv_close(cd);
    utf8.resize(utf8.size() - outbytesleft); // 调整大小以去除未使用的空间
    return { utf8.begin(), utf8.end() };
}

const char *kgtFile = "/Volumes/HP_P800/Gamaker/dong_dong_never_die_170804/GAME/GAME.kgt";

int main() {
    long offset = 0;
    _2dfm::KgtFileHeader header;
    auto file = fopen(kgtFile, "r");
    if (!file) {
        return -1;
    }

    fread(&header, sizeof(_2dfm::KgtFileHeader), 1, file);

    cout << "signature: "
        << header.fileSignature
        << ", projectName: "
        << header.projectName.name
        << endl;

//    offset += 16 + 256;
//    fseek(file, offset, SEEK_SET);
    int intBuffer;
    fread(&intBuffer, sizeof(int), 1, file);
    const int scriptSize = 39;
    _2dfm::ScriptPart sp;
    sp.scriptCount = intBuffer;
    sp.scripts = static_cast<_2dfm::Script *>(malloc(scriptSize * sp.scriptCount));
    fread(sp.scripts, scriptSize, sp.scriptCount, file);
    offset = ftell(file);

    cout << "脚本项共" << sp.scriptCount << "个。" << endl;
    _2dfm::Script *p;
    for (int i = 0; i < sp.scriptCount; ++i) {
        p = reinterpret_cast<_2dfm::Script *>(reinterpret_cast<byte *>(sp.scripts) + i * scriptSize);
        cout << i << '\t' << p->scriptIndex << '\t' << gb2312ToUtf8(p->scriptName) << endl;
    }

    free(sp.scripts);
    return fclose(file);
}
