#include "internal.hpp"

Kgt2dfmStage* kgt_read_stage(Kgt2dfmStream* stream) {
    if (!stream) return nullptr;

    RawFileHeader header;
    stream_read(stream, &header, KGT_FILE_HEADER_SIZE);

    auto* stage = static_cast<Kgt2dfmStage*>(std::calloc(1, sizeof(Kgt2dfmStage)));
    if (!stage) return nullptr;

    std::memcpy(stage->stageName, header.name, 256);

    if (!readCommonResourceFromStream(stream, &stage->common)) {
        kgt_free_stage(stage);
        return nullptr;
    }

    // 跳过 4 字节
    stream_skip(stream, 4);

    // 读取场景配置
    stream_read(stream, &stage->config, sizeof(Kgt2dfmStageConfig));

    return stage;
}

void kgt_free_stage(Kgt2dfmStage* stage) {
    if (!stage) return;
    freeCommonResource(&stage->common);
    std::free(stage);
}
