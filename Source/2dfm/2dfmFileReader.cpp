#include "2dfmFileReader.hpp"
#include "2dfm_reader.h"
#include "2dfmCommon.hpp"
#include "KgtPalette.hpp"
#include "engine/MyString.hpp"
#include "engine/SoundClip.hpp"

#include <axmol.h>

#include <string>
#include <cstring>

#include "game/GameConfig.hpp"
#include "game/ResourcePool.hpp"

/// 将 Kgt2dfmPicture 包装为 _2dfm::Picture 视图（content 指向库内数据，调用方保证生命周期）
static _2dfm::Picture makePictureView(const Kgt2dfmPicture& libPic) {
    _2dfm::Picture view{};
    std::memcpy(&view.header, &libPic.header, sizeof(_2dfm::PictureHeader));
    view.content = const_cast<byte*>(libPic.content);
    return view;
}

/// 从 Kgt2dfmCommonResource 设置游戏 CommonResource
/// 注意：调用后，CR 的 pictures/sounds 数据已被消耗（复制），原库数据可以安全释放
void setCommonResource(CommonResource *result, Kgt2dfmCommonResource *cr) {
    // ── 1. 复制脚本信息 ──
    result->scripts.reserve(cr->scriptCount);
    for (int32_t i = 0; i < cr->scriptCount; ++i) {
        const auto& src = cr->scripts[i];
        result->scripts.emplace_back(KgtScript{
            static_cast<ScriptSpecialFlag>(src.flags),
            gbkToUtf8(src.name),
            static_cast<int>(src.startIdx),
            static_cast<int>(src.endIdx)
        });
    }

    // ── 2. 复制脚本项信息 ──
    result->scriptItems.reserve(cr->scriptItemCount);
    for (int32_t i = 0; i < cr->scriptItemCount; ++i) {
        result->scriptItems.emplace_back(
            new _2dfm::ScriptItem(*reinterpret_cast<_2dfm::ScriptItem*>(&cr->scriptItems[i])));
    }

    // ── 3. 读取创建共享调色盘 ──
    for (int i = 0; i < 8; ++i) {
        result->sharedPalettes[i] = createPalette(
            reinterpret_cast<_2dfm::ColorBgra*>(cr->sharedPalettes[i]), false);
    }

    // ── 4. 读取创建精灵帧对象 ──
    result->spriteFrames.reserve(cr->pictureCount);
    for (int32_t i = 0; i < cr->pictureCount; ++i) {
        auto view = makePictureView(cr->pictures[i]);
        auto& sfi = result->spriteFrames.emplace_back();
        sfi.setFrom2dfmPicture(&view);
        sfi.setSharedPalettes(result->sharedPalettes);
    }

    // ── 5. 声音片段 ──
    result->sounds.reserve(cr->soundCount);
    std::string fmtTemplate;
    if (typeid(*result) == typeid(KgtGame)) {
        fmtTemplate = "kgt/{}/sound/{}.wav";
    } else if (typeid(*result) == typeid(KgtDemo)) {
        fmtTemplate = "demo/{}/sound/{}.wav";
    } else if (typeid(*result) == typeid(KgtPlayer)) {
        fmtTemplate = "player/{}/sound/{}.wav";
    } else {
        fmtTemplate = "stage/{}/sound/{}.wav";
    }
    for (int32_t i = 0; i < cr->soundCount; ++i) {
        const auto& libSnd = cr->sounds[i];

        auto copySound = new _2dfm::Sound();
        copySound->header = *reinterpret_cast<const _2dfm::SoundItemHeader*>(&libSnd.header);
        auto content = static_cast<byte*>(std::malloc(libSnd.header.size));
        std::memcpy(content, libSnd.content, libSnd.header.size);
        copySound->content = content;

        SoundClip *sc = new SoundClip(
            copySound,
            std::vformat(fmtTemplate, std::make_format_args(result->serialNo, i)));
        result->sounds.emplace_back(sc);
    }
}

// ==================== KGT 文件读取 ====================

KgtGame *readKgtFile(const std::string& filepath) {
    auto file = fopen(filepath.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("open kgt file failed");
    }

    Kgt2dfmStream stream = kgt_stream_from_file(file);
    Kgt2dfmGame* raw = kgt_read_game(&stream);
    fclose(file);

    if (!raw) {
        throw std::runtime_error("parse kgt file failed");
    }

    auto result = new KgtGame;
    result->serialNo = 0;
    result->projectName = raw->projectName;
    setCommonResource(result, &raw->common);

    result->playerNames.reserve(_2dfm::maxPlayerNum);
    for (int i = 0; i < _2dfm::maxPlayerNum; ++i) {
        result->playerNames.emplace_back(gbkToUtf8(raw->playerNames[i]));
    }
    result->stageNames.reserve(_2dfm::maxStageNum);
    for (int i = 0; i < _2dfm::maxStageNum; ++i) {
        result->stageNames.emplace_back(gbkToUtf8(raw->stageNames[i]));
    }
    result->demoNames.reserve(_2dfm::maxDemoNum);
    for (int i = 0; i < _2dfm::maxDemoNum; ++i) {
        result->demoNames.emplace_back(gbkToUtf8(raw->demoNames[i]));
    }
    result->reactions.reserve(_2dfm::maxReactionNum);
    for (int i = 0; i < _2dfm::maxReactionNum; ++i) {
        Reaction r;
        r.name = gbkToUtf8(raw->reactions[i].name);
        r.isHurtAction = raw->reactions[i].isHurtAction != 0;
        result->reactions.emplace_back(r);
    }
    result->throwReactions.reserve(_2dfm::maxThrowReactionNum);
    for (int i = 0; i < _2dfm::maxThrowReactionNum; ++i) {
        result->throwReactions.emplace_back(gbkToUtf8(raw->throwReactions[i].name));
    }
    result->recoverTimeConfig = *reinterpret_cast<_2dfm::RecoverTimeConfig*>(&raw->recoverTimeConfig);
    result->demoConfig = *reinterpret_cast<_2dfm::GameDemoConfig*>(&raw->demoConfig);

    result->projectBaseConfig.encryptGame = raw->projectBaseConfig.encryptGame;
    result->projectBaseConfig.allowClash = raw->projectBaseConfig.allowClash;
    result->projectBaseConfig.enableStoryMode = raw->projectBaseConfig.enableStoryMode;
    result->projectBaseConfig.enable1V1Mode = raw->projectBaseConfig.enable1V1Mode;
    result->projectBaseConfig.enableTeamMode = raw->projectBaseConfig.enableTeamMode;
    result->projectBaseConfig.showHpAfterHpBar = raw->projectBaseConfig.showHpAfterHpBar;
    result->projectBaseConfig.pressToStart = raw->projectBaseConfig.pressToStart;

    result->charSelectConfig.selectBoxStartPos = ax::Vec2(raw->charSelectConfig.selectBoxStartX, raw->charSelectConfig.selectBoxStartY);
    result->charSelectConfig.playerAvatarIconSize = ax::Vec2(raw->charSelectConfig.iconWidth, raw->charSelectConfig.iconHeight);
    result->charSelectConfig.rowCount = raw->charSelectConfig.rowNum;
    result->charSelectConfig.columnCount = raw->charSelectConfig.columnNum;
    result->charSelectConfig.player1PortraitPos = ax::Vec2(raw->charSelectConfig.player1PortraitX, raw->charSelectConfig.player1PortraitY);
    result->charSelectConfig.player1PortraitOffset = ax::Vec2(raw->charSelectConfig.player1PortraitTeamOffsetX, raw->charSelectConfig.player1PortraitTeamOffsetY);
    result->charSelectConfig.player2PortraitPos = ax::Vec2(raw->charSelectConfig.player2PortraitX, raw->charSelectConfig.player2PortraitY);
    result->charSelectConfig.player2PortraitOffset = ax::Vec2(raw->charSelectConfig.player2PortraitTeamOffsetX, raw->charSelectConfig.player2PortraitTeamOffsetY);

    for (int i = 0; i < _2dfm::maxPlayerNum; ++i) {
        result->playerSelectableInfos[i] = raw->playerSelectableInfos[i];
    }

    kgt_free_game(raw);
    return result;
}

KgtGame *readKgtFile() {
    const GameConfig &gameConfig = GameConfig::getInstance();
    auto kgtFilePath = std::format("{}/{}", gameConfig.getGameBasePath(), gameConfig.getKgtFileName());
    return readKgtFile(kgtFilePath);
}

// ==================== DEMO 文件读取 ====================

KgtDemo *readDemoFile(int demoNo, const std::string &filepath) {
    auto file = fopen(filepath.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("open demo file failed");
    }

    Kgt2dfmStream stream = kgt_stream_from_file(file);
    Kgt2dfmDemo* raw = kgt_read_demo(&stream);
    fclose(file);

    if (!raw) {
        throw std::runtime_error("parse demo file failed");
    }

    auto result = new KgtDemo;
    result->demoName = raw->demoName;
    result->serialNo = demoNo;
    setCommonResource(result, &raw->common);
    result->config = *reinterpret_cast<_2dfm::KgtDemoConfig*>(&raw->config);

    kgt_free_demo(raw);
    return result;
}

KgtDemo *readDemoByNo(int demoNo) {
    auto kgt = ResourcePool::getInstance().getKgtGame();
    auto demoName = kgt->demoNames[demoNo];
    if (demoName.empty()) {
        return nullptr;
    }
    auto &gameConfig = GameConfig::getInstance();
    const auto fullDemoName = std::format("{}/{}.demo", gameConfig.getGameBasePath(), demoName);
    return readDemoFile(demoNo, fullDemoName);
}

// ==================== Stage 文件读取 ====================

KgtStage *readStageFile(int stageNo, const std::string &filepath) {
    auto file = fopen(filepath.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("open stage file failed");
    }

    Kgt2dfmStream stream = kgt_stream_from_file(file);
    Kgt2dfmStage* raw = kgt_read_stage(&stream);
    fclose(file);

    if (!raw) {
        throw std::runtime_error("parse stage file failed");
    }

    auto result = new KgtStage;
    result->serialNo = stageNo;
    result->stageName = raw->stageName;
    setCommonResource(result, &raw->common);
    result->bgmSoundId = raw->config.bgmSoundId;

    kgt_free_stage(raw);
    return result;
}

KgtStage *readStageByNo(int stageNo) {
    auto kgt = ResourcePool::getInstance().getKgtGame();
    auto stageName = kgt->stageNames[stageNo];
    if (stageName.empty()) {
        return nullptr;
    }
    auto &gameConfig = GameConfig::getInstance();
    const auto fullStageName = std::format("{}/{}.stage", gameConfig.getGameBasePath(), stageName);
    return readStageFile(stageNo, fullStageName);
}

// ==================== Player 文件读取 ====================

KgtPlayer *readPlayerFile(int playerNo, const std::string &filepath) {
    auto file = fopen(filepath.c_str(), "rb");
    if (!file) {
        throw std::runtime_error("open player file failed");
    }

    Kgt2dfmStream stream = kgt_stream_from_file(file);
    Kgt2dfmPlayer* raw = kgt_read_player(&stream);
    fclose(file);

    if (!raw) {
        throw std::runtime_error("parse player file failed");
    }

    auto result = new KgtPlayer;
    result->serialNo = playerNo;
    result->playerName = raw->playerName;
    setCommonResource(result, &raw->common);

    // ── Player 特有数据转换 ──

    // 1. 出招指令
    result->commands.reserve(raw->commandCount);
    for (int32_t i = 0; i < raw->commandCount; ++i) {
        const auto& rawCmd = raw->commands[i];
        PlayerCommand cmd;
        cmd.name = gbkToUtf8(rawCmd.name);
        cmd.timeWindow = rawCmd.timeWindow;
        cmd.airScriptId = rawCmd.airScriptId;
        cmd.standNearScriptId = rawCmd.standNearScriptId;
        cmd.standFarScriptId = rawCmd.standFarScriptId;
        cmd.crouchScriptId = rawCmd.crouchScriptId;

        // 解析指令步骤
        for (int s = 0; s < 10; ++s) {
            int16_t data = rawCmd.cmdData[s];
            auto state = static_cast<CommandStepState>((data >> 12) & 0x03);
            if (state == CommandStepState::INACTIVE || state == CommandStepState::EXCLUDED) {
                continue;
            }
            CommandStep step;
            step.direction = static_cast<CommandDirection>(data & 0x0F);
            step.buttons = static_cast<byte>((data >> 4) & 0x3F);
            step.state = state;
            step.mark = static_cast<CommandStepMark>((data >> 14) & 0x03);
            step.extraData = rawCmd.cmdExtraData[s];
            cmd.steps.push_back(step);
        }
        result->commands.push_back(std::move(cmd));
    }

    // 2. 中招反应绑定
    if (raw->hurtBindCount > 0) {
        result->hurtBinds.assign(raw->hurtBinds, raw->hurtBinds + raw->hurtBindCount);
    }

    // 3. 投掷反应
    result->throwActions.reserve(raw->throwActionCount);
    for (int32_t i = 0; i < raw->throwActionCount; ++i) {
        const auto& rawThrow = raw->throwActions[i];
        ThrowActionInfo info;
        info.picNo = rawThrow.picNo;
        info.offsetX = rawThrow.offsetX;
        info.offsetY = rawThrow.offsetY;
        result->throwActions.push_back(info);
    }

    // 4. AI 档位
    result->aiSlots.reserve(KGT2DFM_AI_SLOT_COUNT);
    for (int i = 0; i < KGT2DFM_AI_SLOT_COUNT; ++i) {
        const auto& rawAi = raw->aiSlots[i];
        AiSlotInfo slot;
        slot.name = gbkToUtf8(rawAi.name);
        slot.airFlag = rawAi.aiAirFlag;
        slot.probability = rawAi.aiProbability;
        slot.rangeMin = rawAi.rangeMin;
        slot.rangeMax = rawAi.rangeMax;

        for (int c = 0; c < KGT2DFM_AI_SLOT_CMD_COUNT; ++c) {
            const auto& rawAiCmd = rawAi.aiCmds[c];
            AiCommandEntry entry;
            entry.dir = rawAiCmd.dir;
            entry.isEnd = rawAiCmd.flag != 0;
            entry.cmdIdx = rawAiCmd.cmdIdx;
            entry.delayTime = rawAiCmd.delayTime;
            slot.commands.push_back(entry);
            if (entry.isEnd) {
                break;
            }
        }
        result->aiSlots.push_back(std::move(slot));
    }

    // 5. 角色属性
    result->stats.age = raw->age;
    result->stats.sexFlag = raw->sexFlag;
    result->stats.hpYPos = raw->statsConfig.hpYPos;
    result->stats.nearThreshold = raw->statsConfig.nearThreshold;
    result->stats.defenceAdjustRate = raw->statsConfig.defenceAdjustRate;
    result->stats.hpAdjustStartPos = raw->statsConfig.hpAdjustStartPos;
    result->stats.hpAdjustRate = raw->statsConfig.hpAdjustRate;
    result->stats.comboAdjustRate = raw->statsConfig.comboAdjustRate;
    result->stats.defenceKey = raw->statsConfig.defenceKey;
    result->stats.hp = raw->statsConfig.hp;
    result->stats.sp = raw->statsConfig.sp;
    result->stats.maxSpSlot = raw->statsConfig.maxSpSlot;
    result->stats.playerFlags = raw->statsConfig.playerFlags;
    result->stats.spAddWhenIAtk = raw->statsConfig.spAddWhenIAtk;
    result->stats.spAddWhenEnemyAtk = raw->statsConfig.spAddWhenEnemyAtk;
    result->stats.initSpSlot = raw->statsConfig.initSpSlot;

    // 6. 故事条目
    result->storyItems.reserve(KGT2DFM_STORY_ITEM_COUNT);
    for (int i = 0; i < KGT2DFM_STORY_ITEM_COUNT; ++i) {
        const auto& rawStory = raw->storyItems[i];
        StoryItem item;
        item.type = static_cast<StoryItemType>(rawStory.itemType);

        switch (item.type) {
            case StoryItemType::BATTLE: {
                auto* battle = reinterpret_cast<const Kgt2dfmStoryCpuConfig*>(&rawStory.rawData[0]);
                // byte 1-2: stageId, winRoundCount
                const uint8_t* d = rawStory.rawData;
                item.battleInfo.stageId = d[0];
                item.battleInfo.winRoundCount = d[1];
                item.battleInfo.hpRecoverRate = d[4];
                item.battleInfo.startRoundNum = d[5];
                item.battleInfo.roundTime = *reinterpret_cast<const int16_t*>(d + 6);
                item.battleInfo.playerStartXPos = *reinterpret_cast<const int16_t*>(d + 8);
                item.battleInfo.flags = *reinterpret_cast<const int32_t*>(d + 12);
                item.battleInfo.compareWithWhoWhenTimeout = d[16];
                item.battleInfo.winPointGetWhenTimeout = d[17];
                item.battleInfo.whoWillGotPointWhenKo = d[18];
                item.battleInfo.winPointGetWhenKo = d[19];

                const auto* cpuCfgs = reinterpret_cast<const Kgt2dfmStoryCpuConfig*>(d + 24);
                for (int c = 0; c < 7; ++c) {
                    const auto& src = cpuCfgs[c];
                    auto& dst = item.battleInfo.cpuConfigs[c];
                    dst.flags = src.flags;
                    dst.cpuId = src.cpuId;
                    dst.cpuLevel = src.cpuLevel;
                    dst.atkBitmap = src.atkBitmap;
                    dst.startXPos = src.startXPos;
                    dst.showTimeSecond = src.showTimeSecond;
                    dst.showTimeExtraRandomSecond = src.showTimeExtraRandomSecond;
                    dst.showWhenWhosHpIsShort = src.showWhenWhosHpIsShort;
                    dst.showWhenWhosHpIsLessThan = src.showWhenWhosHpIsLessThan;
                    dst.winPoint = src.winPoint;
                    dst.addHpWhenKo = src.addHpWhenKo;
                    dst.addSpWhenKo = src.addSpWhenKo;
                    dst.winPointTarget = src.winPointTarget;
                    dst.compareWithWhoWhenTimeout = src.compareWithWhoWhenTimeout;
                    dst.winPointGetWhenTimeout = src.winPointGetWhenTimeout;
                }
                break;
            }
            case StoryItemType::DEMO: {
                item.demoInfo.demoId = rawStory.rawData[0];
                break;
            }
            case StoryItemType::SWITCH: {
                const uint8_t* d = rawStory.rawData;
                item.switchInfo.conditionType = d[0];
                item.switchInfo.conditionValue = d[1];
                item.switchInfo.jumpOffset = d[4];
                break;
            }
            default:
                break;
        }
        result->storyItems.push_back(std::move(item));
    }

    kgt_free_player(raw);
    return result;
}

KgtPlayer *readPlayerByNo(int playerNo) {
    auto kgt = ResourcePool::getInstance().getKgtGame();
    auto playerName = kgt->playerNames[playerNo];
    if (playerName.empty()) {
        return nullptr;
    }
    auto &gameConfig = GameConfig::getInstance();
    const auto fullPlayerName = std::format("{}/{}.player", gameConfig.getGameBasePath(), playerName);
    return readPlayerFile(playerNo, fullPlayerName);
}

// ==================== 纹理创建 ====================

void createTexturesForCommonResource(CommonResource *cr, int paletteNo) {
    for (auto tex : cr->pictures) {
        if (tex) {
            tex->release();
        }
    }
    cr->pictures.clear();
    for (auto &sf : cr->spriteFrames) {
        cr->pictures.emplace_back(sf.createTexture(paletteNo));
    }
}

// ==================== 调色板创建 ====================

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
