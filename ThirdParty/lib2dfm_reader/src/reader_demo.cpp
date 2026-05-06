#include "internal.hpp"

Kgt2dfmDemo* kgt_read_demo(Kgt2dfmStream* stream) {
    if (!stream) return nullptr;

    RawFileHeader header;
    stream_read(stream, &header, KGT_FILE_HEADER_SIZE);

    auto* demo = static_cast<Kgt2dfmDemo*>(std::calloc(1, sizeof(Kgt2dfmDemo)));
    if (!demo) return nullptr;

    std::memcpy(demo->demoName, header.name, 256);

    if (!readCommonResourceFromStream(stream, &demo->common)) {
        kgt_free_demo(demo);
        return nullptr;
    }

    // 跳过 4 字节
    stream_skip(stream, 4);

    // 读取 DEMO 配置
    stream_read(stream, &demo->config, sizeof(Kgt2dfmDemoConfig));

    return demo;
}

void kgt_free_demo(Kgt2dfmDemo* demo) {
    if (!demo) return;
    freeCommonResource(&demo->common);
    std::free(demo);
}
