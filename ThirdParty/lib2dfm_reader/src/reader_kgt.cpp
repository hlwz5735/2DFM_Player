#include "internal.hpp"

Kgt2dfmGame* kgt_read_game(Kgt2dfmStream* stream) {
    if (!stream) return nullptr;

    // 读取文件头
    RawFileHeader header;
    stream_read(stream, &header, KGT_FILE_HEADER_SIZE);

    // 读取公共资源
    auto* game = static_cast<Kgt2dfmGame*>(std::calloc(1, sizeof(Kgt2dfmGame)));
    if (!game) return nullptr;

    std::memcpy(game->projectName, header.name, 256);

    if (!readCommonResourceFromStream(stream, &game->common)) {
        kgt_free_game(game);
        return nullptr;
    }

    // 跳过 4 字节
    stream_skip(stream, 4);

    // 读取角色名称 (50 个)
    RawNameInfo namesBuf[MAX_PLAYER_NUM];
    stream_read(stream, namesBuf, sizeof(RawNameInfo) * MAX_PLAYER_NUM);
    for (int i = 0; i < MAX_PLAYER_NUM; ++i) {
        std::memcpy(game->playerNames[i], namesBuf[i].name, 256);
    }

    // 读取受击反应 (200 个)
    RawReactionItem reactions[MAX_REACTION_NUM];
    stream_read(stream, reactions, sizeof(RawReactionItem) * MAX_REACTION_NUM);
    for (int i = 0; i < MAX_REACTION_NUM; ++i) {
        std::memcpy(game->reactions[i].name, reactions[i].reactionName, 32);
        game->reactions[i].isHurtAction = reactions[i].isHurtAction;
    }

    // 跳过 4 字节 + 读取硬直时间配置
    stream_skip(stream, 4);
    stream_read_val(stream, game->recoverTimeConfig);

    // 读取场景名称 (50 个)
    RawNameInfo stageNamesBuf[MAX_STAGE_NUM];
    stream_read(stream, stageNamesBuf, sizeof(RawNameInfo) * MAX_STAGE_NUM);
    for (int i = 0; i < MAX_STAGE_NUM; ++i) {
        std::memcpy(game->stageNames[i], stageNamesBuf[i].name, 256);
    }

    // 读取影片名称 (100 个)
    RawNameInfo demoNamesBuf[MAX_DEMO_NUM];
    stream_read(stream, demoNamesBuf, sizeof(RawNameInfo) * MAX_DEMO_NUM);
    for (int i = 0; i < MAX_DEMO_NUM; ++i) {
        std::memcpy(game->demoNames[i], demoNamesBuf[i].name, 256);
    }

    // 读取 DEMO 配置
    stream_read(stream, &game->demoConfig, DEMO_CONFIG_SIZE);

    // 读取项目基础配置
    RawProjectBaseConfig pbc;
    stream_read_val(stream, pbc);
    game->projectBaseConfig.encryptGame      = (pbc.rawValue & 0x01) ? 1 : 0;
    game->projectBaseConfig.allowClash       = (pbc.rawValue & 0x02) ? 1 : 0;
    game->projectBaseConfig.enableStoryMode  = (pbc.rawValue & 0x04) ? 1 : 0;
    game->projectBaseConfig.enable1V1Mode    = (pbc.rawValue & 0x08) ? 1 : 0;
    game->projectBaseConfig.enableTeamMode   = (pbc.rawValue & 0x10) ? 1 : 0;
    game->projectBaseConfig.showHpAfterHpBar = (pbc.rawValue & 0x20) ? 1 : 0;
    game->projectBaseConfig.pressToStart     = (pbc.rawValue & 0x40) ? 1 : 0;

    // 读取投掷反应 (200 个)
    RawThrowReaction throwReactions[MAX_THROW_REACTION_NUM];
    stream_read(stream, throwReactions, sizeof(RawThrowReaction) * MAX_THROW_REACTION_NUM);
    for (int i = 0; i < MAX_THROW_REACTION_NUM; ++i) {
        std::memcpy(game->throwReactions[i].name, throwReactions[i].name, 32);
    }

    // 跳过位置数据 (264 字节)
    stream_skip(stream, 264);

    // 读取角色选择配置
    stream_read(stream, &game->charSelectConfig, sizeof(Kgt2dfmCharSelectConfig));

    // 读取角色可选择性信息
    stream_read(stream, game->playerSelectableInfos, MAX_PLAYER_NUM);

    return game;
}

void kgt_free_game(Kgt2dfmGame* game) {
    if (!game) return;
    freeCommonResource(&game->common);
    std::free(game);
}
