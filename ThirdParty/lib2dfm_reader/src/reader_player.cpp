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

    // ── 1. 读取出招指令列表 ──
    int32_t intBuf = 0;
    stream_read_val(stream, intBuf);
    player->commandCount = intBuf;
    if (intBuf > 0) {
        player->commands = static_cast<Kgt2dfmCommandItem*>(
            std::malloc(sizeof(Kgt2dfmCommandItem) * intBuf));
        if (!player->commands) { kgt_free_player(player); return nullptr; }
        stream_read(stream, player->commands, sizeof(Kgt2dfmCommandItem) * intBuf);
    }

    // ── 2. 读入中招反应绑定 ──
    stream_read_val(stream, intBuf);
    player->hurtBindCount = intBuf;
    if (intBuf > 0) {
        player->hurtBinds = static_cast<int32_t*>(
            std::malloc(sizeof(int32_t) * intBuf));
        if (!player->hurtBinds) { kgt_free_player(player); return nullptr; }
        stream_read(stream, player->hurtBinds, sizeof(int32_t) * intBuf);
    }

    // ── 3. 读入投掷反应 ──
    stream_read_val(stream, intBuf);
    player->throwActionCount = intBuf;
    if (intBuf > 0) {
        player->throwActions = static_cast<Kgt2dfmPlayerThrowAction*>(
            std::malloc(sizeof(Kgt2dfmPlayerThrowAction) * intBuf));
        if (!player->throwActions) { kgt_free_player(player); return nullptr; }
        stream_read(stream, player->throwActions, sizeof(Kgt2dfmPlayerThrowAction) * intBuf);
    }

    // ── 4. 跳过 10 字节未知数据 ──
    stream_skip(stream, 10);

    // ── 5. 读入 AI 档位 (固定 100 个) ──
    stream_read(stream, player->aiSlots, sizeof(Kgt2dfmAiSlot) * KGT2DFM_AI_SLOT_COUNT);

    // ── 6. 跳过 86 字节未知数据 ──
    stream_skip(stream, 86);

    // ── 7. 读入角色基础属性 ──
    stream_read_val(stream, player->age);
    stream_read_val(stream, player->sexFlag);

    // ── 8. 跳过 1740 字节未知数据 ──
    stream_skip(stream, 1740);

    // ── 9. 读入角色战斗属性 ──
    stream_read(stream, &player->statsConfig, sizeof(Kgt2dfmPlayerStatsConfig));

    // ── 10. 读入故事模式条目 (固定 100 个) ──
    stream_read(stream, player->storyItems, sizeof(Kgt2dfmStoryItemRaw) * KGT2DFM_STORY_ITEM_COUNT);

    return player;
}

void kgt_free_player(Kgt2dfmPlayer* player) {
    if (!player) return;
    freeCommonResource(&player->common);
    std::free(player->commands);
    std::free(player->hurtBinds);
    std::free(player->throwActions);
    std::free(player);
}
