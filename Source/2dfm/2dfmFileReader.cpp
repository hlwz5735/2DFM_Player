#include "2dfmFileReader.hpp"
#include "2dfmCommon.hpp"
#include "KgtPalette.hpp"
#include "engine/SoundClip.hpp"
#include "engine/MyString.hpp"
#include <axmol.h>

namespace {
    void readScripts(CommonResource *result, byte *rawData, int scriptCount, int itemCount) {
        for (auto i = 0; i < scriptCount; ++i) {
            auto s = reinterpret_cast<_2dfm::Script *>(rawData + i * _2dfm::SCRIPT_SIZE);
            int endIndex = 0;
            if (i == scriptCount - 1) {
                endIndex = itemCount;
            } else {
                auto nextS = reinterpret_cast<_2dfm::Script *>(rawData + (i + 1) * _2dfm::SCRIPT_SIZE);
                endIndex = nextS->scriptIndex;
            }
            KgtScript kgtScript {
                    static_cast<ScriptSpecialFlag>(s->flags),
                    gbkToUtf8(s->scriptName),
                    static_cast<int>(s->scriptIndex),
                    endIndex
            };
            result->scripts.emplace_back(kgtScript);
        }
    }
}

/// 设置公共资源部分
/// 注意：只能初次设置，如果已有数据，调用时会导致内存泄漏
void setCommonResource(CommonResource *result, const _2dfm::CommonResourcePart &commonResource) {
    // 复制脚本信息
    result->scripts.reserve(commonResource.scriptCount);
    readScripts(result, commonResource.rawScriptsData, commonResource.scriptCount, commonResource.scriptItemCount);

    // 复制脚本项信息
    result->scriptItems.reserve(commonResource.scriptItemCount);
    for (auto i = 0; i < commonResource.scriptItemCount; ++i) {
        result->scriptItems.emplace_back(new _2dfm::ScriptItem(commonResource.scriptItems[i]));
    }

    // 读取创建共享调色盘
    for (int i = 0; i < 8; ++i) {
        result->sharedPalettes[i] = createPalette(commonResource.sharedPalettes[i], false);
    }

    // 读取创建精灵帧对象
    result->spriteFrames.reserve(commonResource.pictureCount);
    for (_2dfm::Picture *picture : commonResource.pictures) {
        auto &sfi = result->spriteFrames.emplace_back();
        sfi.setFrom2dfmPicture(picture);
        sfi.setSharedPalettes(result->sharedPalettes);
    }

    // 声音片段
    result->sounds.reserve(commonResource.soundCount);
    for (auto s : commonResource.sounds) {
        result->sounds.emplace_back(SoundClip::from2dfmSound(s));
    }
}

KgtGame *readKgtFile(const std::string& filepath) {
    _2dfm::KgtFileHeader header;
    // 打开文件
    auto file = fopen(filepath.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("open kgt file failed");
    }
    // 读入文件头
    fread(&header, _2dfm::KGT_FILE_HEADER_SIZE, 1, file);
    long offset = ftell(file);

    // 读入公共资源部分
    auto commonResource = readCommonResourcePart(&offset, file);

    // 读入玩家名称信息
    fseek(file, offset + 4, SEEK_SET);
    _2dfm::NameInfo *playerNames = static_cast<_2dfm::NameInfo *>(malloc(sizeof(_2dfm::NameInfo) * _2dfm::maxPlayerNum));
    fread(playerNames, sizeof(_2dfm::NameInfo), _2dfm::maxPlayerNum, file);

    // 读入受击反应信息
    _2dfm::ReactionItem *reactionItems = static_cast<_2dfm::ReactionItem *>(malloc(sizeof(_2dfm::ReactionItem) * _2dfm::maxReactionNum));
    fread(reactionItems, sizeof(_2dfm::ReactionItem), _2dfm::maxReactionNum, file);

    // 读入硬直时间信息
    fseek(file, 4, SEEK_CUR);
    _2dfm::RecoverTimeConfig recoverTimeConfig;
    fread(&recoverTimeConfig, sizeof(_2dfm::RecoverTimeConfig), 1, file);

    // 读入场景信息
    _2dfm::NameInfo *stageNames = static_cast<_2dfm::NameInfo *>(malloc(sizeof(_2dfm::NameInfo) * _2dfm::maxStageNum));
    fread(stageNames, sizeof(_2dfm::NameInfo), _2dfm::maxStageNum, file);

    // 读入影片信息
    _2dfm::NameInfo *demoNames = static_cast<_2dfm::NameInfo *>(malloc(sizeof(_2dfm::NameInfo) * _2dfm::maxDemoNum));
    fread(demoNames, sizeof(_2dfm::NameInfo), _2dfm::maxDemoNum, file);

    // 读入影片信息
    _2dfm::GameDemoConfig demoConfig;
    fread(&demoConfig, _2dfm::DEMO_CONFIG_SIZE, 1, file);

    // 读入项目通常设置
    _2dfm::ProjectBaseConfig pbc;
    fread(&pbc, 4, 1, file);

    // 读入投掷反应列表
    _2dfm::ThrowReaction *throwReactions = static_cast<_2dfm::ThrowReaction *>(malloc(sizeof(_2dfm::ThrowReaction) * _2dfm::maxThrowReactionNum));
    fread(throwReactions, sizeof(_2dfm::ThrowReaction), _2dfm::maxThrowReactionNum, file);

    // 跳过位置数据
    fseek(file, 264, SEEK_CUR);
    _2dfm::CharSelectConfig csc;
    fread(&csc, sizeof(_2dfm::CharSelectConfig), 1, file);

    // 角色可选择性信息
    byte playerSelectableInfos[_2dfm::maxPlayerNum];
    fread(playerSelectableInfos, sizeof(byte), _2dfm::maxPlayerNum, file);

    // 拼装数据
    auto result = new KgtGame;
    result->projectName = header.name.name;
    setCommonResource(result, commonResource);

    result->playerNames.reserve(_2dfm::maxPlayerNum);
    for (auto playerName = playerNames; playerName != playerNames + _2dfm::maxPlayerNum; ++playerName) {
        result->playerNames.emplace_back(gbkToUtf8(playerName->name));
    }
    result->stageNames.reserve(_2dfm::maxStageNum);
    for (auto stageName = stageNames; stageName != stageNames + _2dfm::maxStageNum; ++stageName) {
        result->stageNames.emplace_back(gbkToUtf8(stageName->name));
    }
    result->demoNames.reserve(_2dfm::maxDemoNum);
    for (auto demoName = demoNames; demoName != demoNames + _2dfm::maxDemoNum; ++demoName) {
        result->demoNames.emplace_back(gbkToUtf8(demoName->name));
    }
    result->reactions.reserve(_2dfm::maxReactionNum);
    for (auto reactionItem = reactionItems; reactionItem != reactionItems + _2dfm::maxReactionNum; ++reactionItem) {
        Reaction r;
        r.name = gbkToUtf8(reactionItem->reactionName);
        r.isHurtAction = static_cast<bool>(reactionItem->isHurtAction);
        result->reactions.emplace_back(r);
    }
    result->throwReactions.reserve(_2dfm::maxThrowReactionNum);
    for (auto reaction = throwReactions; reaction != throwReactions + _2dfm::maxThrowReactionNum; ++reaction) {
        result->throwReactions.emplace_back(gbkToUtf8(reaction->name));
    }
    result->recoverTimeConfig = recoverTimeConfig;
    result->demoConfig = demoConfig;

    // 游戏基础设置
    result->projectBaseConfig.encryptGame = pbc.value.encryptGame;
    result->projectBaseConfig.allowClash = pbc.value.allowClash;
    result->projectBaseConfig.enableStoryMode = pbc.value.enableStoryMode;
    result->projectBaseConfig.enable1V1Mode = pbc.value.enable1V1Mode;
    result->projectBaseConfig.enableTeamMode = pbc.value.enableTeamMode;
    result->projectBaseConfig.showHpAfterHpBar = pbc.value.showHpAfterHpBar;
    result->projectBaseConfig.pressToStart = pbc.value.pressToStart;

    // 角色选择画面设置
    result->charSelectConfig.selectBoxStartPos = ax::Vec2(csc.selectBoxStartX, csc.selectBoxStartY);
    result->charSelectConfig.playerAvatarIconSize = ax::Vec2(csc.iconWidth, csc.iconHeight);
    result->charSelectConfig.rowCount = csc.rowNum;
    result->charSelectConfig.columnCount = csc.columnNum;
    result->charSelectConfig.player1PortraitPos = ax::Vec2(csc.player1PortraitX, csc.player1PortraitY);
    result->charSelectConfig.player1PortraitOffset = ax::Vec2(csc.player1PortraitTeamOffsetX, csc.player1PortraitTeamOffsetY);
    result->charSelectConfig.player2PortraitPos = ax::Vec2(csc.player2PortraitX, csc.player2PortraitY);
    result->charSelectConfig.player2PortraitOffset = ax::Vec2(csc.player2PortraitTeamOffsetX, csc.player2PortraitTeamOffsetY);

    // 角色可选择性信息
    for (auto i = 0; i < _2dfm::maxPlayerNum; ++i) {
        result->playerSelectableInfos[i] = playerSelectableInfos[i];
    }

    // 资源清理
    fclose(file);
    free(playerNames);
    free(stageNames);
    free(demoNames);
    free(reactionItems);
    free(throwReactions);
    freeCommonResourcePart(&commonResource);

    return result;
}

KgtDemo *readDemoFile(const std::string_view filepath) {
    _2dfm::KgtFileHeader header;
    // 打开文件
    auto file = fopen(filepath.data(), "rb");
    if (!file) {
        throw std::runtime_error("open demo file failed");
    }
    // 读入文件头
    fread(&header, _2dfm::KGT_FILE_HEADER_SIZE, 1, file);
    long offset = ftell(file);

    // 读入公共资源部分
    auto commonResource = readCommonResourcePart(&offset, file);
    fseek(file, offset + 4, SEEK_SET);

    // 读入DEMO配置信息
    _2dfm::KgtDemoConfig config;
    fread(&config, sizeof(_2dfm::KgtDemoConfig), 1, file);

    auto result = new KgtDemo;
    // 资源拼接部分
    result->demoName = header.name.name;
    setCommonResource(result, commonResource);
    result->config = config;

    // 资源清理
    fclose(file);
    freeCommonResourcePart(&commonResource);

    return result;
}

KgtStage *readStageFile(const std::string &filepath) {
    _2dfm::KgtFileHeader header;
    // 打开文件
    auto file = fopen(filepath.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("open stage file failed");
    }
    // 读入文件头
    fread(&header, _2dfm::KGT_FILE_HEADER_SIZE, 1, file);
    long offset = ftell(file);

    // 读入公共资源部分
    auto commonResource = readCommonResourcePart(&offset, file);
    fseek(file, offset + 4, SEEK_SET);

    // 读入场景配置信息
    _2dfm::KgtStageConfig config;
    fread(&config, sizeof(_2dfm::KgtStageConfig), 1, file);

    auto result = new KgtStage;
    // 资源拼接部分
    result->stageName = header.name.name;
    setCommonResource(result, commonResource);
    result->bgmSoundId = config.bgmSoundId;

    // 资源清理
    fclose(file);
    freeCommonResourcePart(&commonResource);

    return result;
}

KgtPlayer *readPlayerFile(const std::string &filepath) {
    _2dfm::KgtFileHeader header;
    // 打开文件
    auto file = fopen(filepath.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("open player file failed");
    }
    // 读入文件头
    fread(&header, _2dfm::KGT_FILE_HEADER_SIZE, 1, file);
    long offset = ftell(file);

    // 读入公共资源部分
    auto commonResource = readCommonResourcePart(&offset, file);
    fseek(file, offset + 4, SEEK_SET);

    // 读入角色文件特有数据
    // TODO: 受伤动作绑定
    // TODO: 投掷动作绑定
    // TODO: 出招指令信息
    // TODO: AI出招条目
    // TODO: 故事条目信息

    auto result = new KgtPlayer;
    // 资源拼接部分
    result->playerName = header.name.name;
    setCommonResource(result, commonResource);

    // 资源清理
    fclose(file);
    freeCommonResourcePart(&commonResource);

    return result;
}

void createTexturesForCommonResource(CommonResource *cr, int paletteNo) {
    for (auto tex : cr->pictures) {
        delete tex;
    }
    cr->pictures.clear();
    for (auto &sf : cr->spriteFrames) {
        cr->pictures.emplace_back(sf.createTexture(paletteNo));
    }
}

_2dfm::CommonResourcePart readCommonResourcePart(long *offset, FILE *file) {
    fseek(file, *offset, SEEK_SET);

    // 读入脚本项列表
    int intBuffer;
    fread(&intBuffer, sizeof(int), 1, file);
    _2dfm::CommonResourcePart crp;
    crp.scriptCount = intBuffer;
    // 这里不能直接按指针相加读取，因为存在字节对齐问题
    crp.rawScriptsData = static_cast<byte *>(malloc(_2dfm::SCRIPT_SIZE * crp.scriptCount));
    if (!crp.rawScriptsData) {
        throw std::runtime_error("there is no enough memory space!");
    }
    fread(crp.rawScriptsData, _2dfm::SCRIPT_SIZE, crp.scriptCount, file);

    // 读入脚本格子列表
    fread(&intBuffer, sizeof(int), 1, file);
    crp.scriptItemCount = intBuffer;
    crp.scriptItems = static_cast<_2dfm::ScriptItem *>(malloc(_2dfm::SCRIPT_ITEM_SIZE * crp.scriptItemCount));
    if (!crp.scriptItems) {
        throw std::runtime_error("there is no enough memory space!");
    }
    fread(crp.scriptItems, _2dfm::SCRIPT_ITEM_SIZE, crp.scriptItemCount, file);

    // 读入精灵帧信息
    fread(&intBuffer, sizeof(int), 1, file);
    crp.pictureCount = intBuffer;
    crp.pictures.reserve(crp.pictureCount);
    for (int i = 0; i < crp.pictureCount; ++i) {
        auto sf = static_cast<_2dfm::Picture *>(malloc(sizeof(_2dfm::Picture)));
        fread(&sf->header, _2dfm::PICTUR_HEADER_SIZE, 1, file);
        auto sz = get2dfmPictureSize(&sf->header);
        byte *pSpriteContent = static_cast<byte *>(malloc(sz));
        fread(pSpriteContent, sz, 1, file);
        sf->content = pSpriteContent;
        crp.pictures.emplace_back(sf);
    }

    // 读入调色盘信息
    for (auto &sharedPalette : crp.sharedPalettes) {
        _2dfm::ColorBgra *pPalette = static_cast<_2dfm::ColorBgra *>(malloc(_2dfm::PALETTE_SIZE));
        fread(pPalette, _2dfm::PALETTE_SIZE, 1, file);
        sharedPalette = pPalette;
        fseek(file, sizeof(int) * 8, SEEK_CUR);
    }

    // 读入声音信息
    fread(&intBuffer, sizeof(int), 1, file);
    crp.soundCount = intBuffer;
    crp.sounds.reserve(crp.soundCount);
    for (int i = 0; i < crp.soundCount; ++i) {
        auto s = static_cast<_2dfm::Sound *>(malloc(sizeof(_2dfm::Sound)));
        fread(&(s->header), _2dfm::SOUND_ITEM_HEADER_SIZE, 1, file);
        if (s->header.size > 0) {
            s->content = static_cast<byte *>(malloc(s->header.size));
            fread(s->content, s->header.size, 1, file);
        }
        crp.sounds.emplace_back(s);
    }

    *offset = ftell(file);
    return crp;
}

KgtPalette *createPalette(_2dfm::ColorBgra* originPalette, bool isPrivate) {
    auto palette = new KgtPalette();

    for (int i = 0; i < 256; ++i) {
        const auto& color = originPalette[i];
        palette->colors[i] = color;

        if (color.channel.red == 0 && color.channel.green == 0 && color.channel.blue == 0) {
            palette->colors[i].channel.alpha = 0;
        } else {
            if (isPrivate) {
                palette->colors[i].channel.alpha = 255;
            } else {
                palette->colors[i].channel.alpha = color.channel.alpha == 1 ? 255 : 0;
            }
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
