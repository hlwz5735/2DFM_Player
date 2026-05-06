#include "internal.hpp"

bool readCommonResourceFromStream(Kgt2dfmStream* stream, Kgt2dfmCommonResource* out) {
    std::memset(out, 0, sizeof(Kgt2dfmCommonResource));

    int32_t intBuf = 0;

    // ── 读取脚本列表 ──
    stream_read_val(stream, intBuf);
    out->scriptCount = intBuf;

    // 读取原始脚本数据
    std::vector<uint8_t> rawScripts(SCRIPT_SIZE * out->scriptCount);
    stream_read(stream, rawScripts.data(), rawScripts.size());

    // ── 读取脚本项列表 ──
    stream_read_val(stream, intBuf);
    out->scriptItemCount = intBuf;
    out->scriptItems = static_cast<Kgt2dfmScriptItem*>(
        std::malloc(sizeof(Kgt2dfmScriptItem) * out->scriptItemCount));
    if (!out->scriptItems) return false;
    stream_read(stream, out->scriptItems, SCRIPT_ITEM_SIZE * out->scriptItemCount);

    // ── 解析脚本信息 (需要 scriptItemCount 来计算 endIdx) ──
    out->scripts = static_cast<Kgt2dfmScript*>(
        std::malloc(sizeof(Kgt2dfmScript) * out->scriptCount));
    if (!out->scripts) return false;

    for (int32_t i = 0; i < out->scriptCount; ++i) {
        auto* raw = reinterpret_cast<RawScript*>(rawScripts.data() + i * SCRIPT_SIZE);
        auto& s = out->scripts[i];
        std::memcpy(s.name, raw->scriptName, 32);
        s.flags = raw->flags;
        s.startIdx = static_cast<int32_t>(raw->scriptIndex);

        if (i == out->scriptCount - 1) {
            s.endIdx = out->scriptItemCount;
        } else {
            auto* nextRaw = reinterpret_cast<RawScript*>(rawScripts.data() + (i + 1) * SCRIPT_SIZE);
            s.endIdx = static_cast<int32_t>(nextRaw->scriptIndex);
        }
    }

    // ── 读取图片列表 ──
    stream_read_val(stream, intBuf);
    out->pictureCount = intBuf;
    out->pictures = static_cast<Kgt2dfmPicture*>(
        std::calloc(out->pictureCount, sizeof(Kgt2dfmPicture)));
    if (!out->pictures && out->pictureCount > 0) return false;

    for (int32_t i = 0; i < out->pictureCount; ++i) {
        auto& pic = out->pictures[i];
        stream_read(stream, &pic.header, PICTURE_HEADER_SIZE);
        int32_t sz = getPictureDataSize(&pic.header);
        pic.contentSize = static_cast<size_t>(sz);
        pic.content = static_cast<uint8_t*>(std::malloc(sz));
        if (!pic.content) return false;
        stream_read(stream, pic.content, sz);
    }

    // ── 读取共享调色盘 (8 个) ──
    for (int i = 0; i < 8; ++i) {
        stream_read(stream, out->sharedPalettes[i], PALETTE_SIZE);
        stream_skip(stream, sizeof(int32_t) * 8); // 跳过额外字段
    }

    // ── 读取声音列表 ──
    stream_read_val(stream, intBuf);
    out->soundCount = intBuf;
    out->sounds = static_cast<Kgt2dfmSound*>(
        std::calloc(out->soundCount, sizeof(Kgt2dfmSound)));
    if (!out->sounds && out->soundCount > 0) return false;

    for (int32_t i = 0; i < out->soundCount; ++i) {
        auto& snd = out->sounds[i];
        stream_read(stream, &snd.header, SOUND_ITEM_HEADER_SIZE);
        snd.contentSize = static_cast<size_t>(snd.header.size);
        if (snd.header.size > 0) {
            snd.content = static_cast<uint8_t*>(std::malloc(snd.header.size));
            if (!snd.content) return false;
            stream_read(stream, snd.content, snd.header.size);
        }
    }

    return true;
}

void freeCommonResource(Kgt2dfmCommonResource* cr) {
    if (!cr) return;

    std::free(cr->scripts);
    cr->scripts = nullptr;

    std::free(cr->scriptItems);
    cr->scriptItems = nullptr;

    if (cr->pictures) {
        for (int32_t i = 0; i < cr->pictureCount; ++i) {
            std::free(cr->pictures[i].content);
        }
        std::free(cr->pictures);
        cr->pictures = nullptr;
    }

    if (cr->sounds) {
        for (int32_t i = 0; i < cr->soundCount; ++i) {
            std::free(cr->sounds[i].content);
        }
        std::free(cr->sounds);
        cr->sounds = nullptr;
    }
}
