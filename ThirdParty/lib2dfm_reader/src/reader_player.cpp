#include "internal.hpp"

Kgt2dfmPlayer* kgt_read_player(Kgt2dfmStream* stream) {
    if (!stream) return nullptr;

    RawFileHeader header;
    stream_read(stream, &header, KGT_FILE_HEADER_SIZE);

    auto* player = static_cast<Kgt2dfmPlayer*>(std::calloc(1, sizeof(Kgt2dfmPlayer)));
    if (!player) return nullptr;

    std::memcpy(player->playerName, header.name, 256);

    if (!readCommonResourceFromStream(stream, &player->common)) {
        kgt_free_player(player);
        return nullptr;
    }

    // 跳过 4 字节
    stream_skip(stream, 4);

    // TODO: 受伤动作绑定
    // TODO: 投掷动作绑定
    // TODO: 出招指令信息
    // TODO: AI出招条目
    // TODO: 故事条目信息

    return player;
}

void kgt_free_player(Kgt2dfmPlayer* player) {
    if (!player) return;
    freeCommonResource(&player->common);
    std::free(player);
}
