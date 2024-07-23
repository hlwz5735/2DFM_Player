#include <stdexcept>
#include <iconv.h>
#include "2dfmFileReader.hpp"
#include "2dfmCommon.hpp"

std::string gb2312ToUtf8(const char* gb2312) {
    auto len = strlen(gb2312);
    if (len == 0) {
        return "";
    }
    std::vector<char> utf8(len * 4); // 分配足够大的空间，防止溢出
    char* inbuf = const_cast<char*>(gb2312);
    size_t inbytesleft = len;
    char* outbuf = &utf8[0];
    size_t outbytesleft = utf8.size();

    iconv_t cd = iconv_open("UTF-8", "GB2312");
    if (cd == (iconv_t) - 1) {
        throw std::runtime_error("iconv_open failed");
    }

    if (iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft) == (size_t) - 1) {
        iconv_close(cd);
        throw std::runtime_error("iconv failed");
    }

    iconv_close(cd);
    utf8.resize(utf8.size() - outbytesleft); // 调整大小以去除未使用的空间
    return { utf8.begin(), utf8.end() };
}

KgtGame readKgtFile(const std::string& filepath) {
    _2dfm::KgtFileHeader header;
    // 打开文件
    auto file = fopen(filepath.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("open kgt file failed");
    }
    // 读入文件头
    fread(&header, sizeof(_2dfm::KgtFileHeader), 1, file);
    // 读入脚本项列表
    int intBuffer;
    fread(&intBuffer, sizeof(int), 1, file);
    _2dfm::ScriptPart sp;
    sp.scriptCount = intBuffer;
    sp.scripts = static_cast<_2dfm::Script *>(malloc(_2dfm::SCRIPT_SIZE * sp.scriptCount));
    if (!sp.scripts) {
        throw std::runtime_error("there is no enough memory space!");
    }
    fread(sp.scripts, _2dfm::SCRIPT_SIZE, sp.scriptCount, file);
    //_2dfm::Script *p;
    //for (int i = 0; i < sp.scriptCount; ++i) {
    //    p = reinterpret_cast<_2dfm::Script *>(
    //        reinterpret_cast<byte *>(sp.scripts) + i * _2dfm::scriptSize
    //    );
    //}

    // 读入脚本格子列表
    fread(&intBuffer, sizeof(int), 1, file);
    sp.scriptItemCount = intBuffer;
    sp.scriptItems = static_cast<_2dfm::ScriptItem *>(malloc(_2dfm::SCRIPT_ITEM_SIZE * sp.scriptItemCount));
    if (!sp.scriptItems) {
        throw std::runtime_error("there is no enough memory space!");
    }
    fread(sp.scriptItems, _2dfm::SCRIPT_ITEM_SIZE, sp.scriptItemCount, file);

    // 读入精灵帧信息
    fread(&intBuffer, sizeof(int), 1, file);
    sp.pictureCount = intBuffer;
    sp.pictures.reserve(sp.pictureCount);
    for (int i = 0; i < sp.pictureCount; ++i) {
        auto sf = new _2dfm::Picture();
        fread(&sf->header, _2dfm::PICTUR_HEADER_SIZE, 1, file);
        auto sz = get2dfmPictureSize(&sf->header);
        byte *pSpriteContent = static_cast<byte *>(malloc(sz));
        fread(pSpriteContent, sz, 1, file);
        sf->content = pSpriteContent;
        sp.pictures.emplace_back(sf);
    }

    // 读入调色盘信息
    for (int i = 0; i < 8; ++i) {
        _2dfm::ColorBgra *pPalette = static_cast<_2dfm::ColorBgra *>(malloc(_2dfm::PALETTE_SIZE));
        fread(pPalette, _2dfm::PALETTE_SIZE, 1, file);
        sp.sharedPalettes.emplace_back(pPalette);
        fseek(file, sizeof(int) * 8, SEEK_CUR);
    }

    // 读入声音信息
    fread(&intBuffer, sizeof(int), 1, file);
    sp.soundCount = intBuffer;
    sp.sounds.reserve(sp.soundCount);
    for (int i = 0; i < sp.soundCount; ++i) {
        auto s = new _2dfm::Sound();
        fread(&s->header, _2dfm::SOUND_ITEM_HEADER_SIZE, 1, file);
        if (s->header.size > 0) {
            s->content = static_cast<byte *>(malloc(s->header.size));
            fread(s->content, s->header.size, 1, file);
        }
        sp.sounds.emplace_back(s);
    }

    
    // 资源清理
    // TODO: destroySp(&sp);
    // free(sp.scripts);
    // free(sp.scriptItems);
    fclose(file);

    // 拼装数据
    KgtGame result;
    result.projectName = header.projectName.name;
    result.sharedPalettes.reserve(8);
    for (int i = 0; i < 8; ++i) {
        result.sharedPalettes.emplace_back(createSdlPalette(sp.sharedPalettes[i]));
    }
    result.spriteFrames.reserve(sp.pictureCount);
    for (auto pi = sp.pictures.begin(); pi != sp.pictures.end(); ++pi) {
        auto &sfi = result.spriteFrames.emplace_back();
        sfi.setFrom2dfmPicture(*pi);
        sfi.setSharedPalettes(result.sharedPalettes.data());
    }

    result.sounds = sp.sounds;

    return result;
}

SDL_Palette *createSdlPalette(_2dfm::ColorBgra *originPalette) {
    auto palette = SDL_AllocPalette(256);

    for (int i = 0; i < 256; ++i) {
        (palette->colors)[i].r = static_cast<Uint8>(originPalette[i].channel.red);
        (palette->colors)[i].g = static_cast<Uint8>(originPalette[i].channel.green);
        (palette->colors)[i].b = static_cast<Uint8>(originPalette[i].channel.blue);
        (palette->colors)[i].a = static_cast<Uint8>(static_cast<unsigned char>(originPalette[i].channel.alpha) == 0 ? 0 : 255);
        if ((palette->colors)[i].r == 0 && (palette->colors)[i].g == 0 && (palette->colors)[i].b == 0) {
            (palette->colors)[i].a = 0;
        }
    }

    return palette;
}

int get2dfmPictureRealSize(_2dfm::PictureHeader *sfh) {
    return sfh->width * sfh->height + (sfh->hasPrivatePalette ? 1024 : 0);
}

int get2dfmPictureSize(_2dfm::PictureHeader *sfh) {
    if (sfh->size == 0) {
        return get2dfmPictureRealSize(sfh);
    } else {
        return sfh->size;
    }
}
