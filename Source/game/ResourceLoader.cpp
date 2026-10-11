//
// ResourceLoader: 2DFM 四种资源文件的唯一读取实现
//

#include "ResourceLoader.hpp"

#include "2dfm/CommonResource.hpp"
#include "2dfm/KgtDemo.hpp"
#include "2dfm/KgtGame.hpp"
#include "2dfm/KgtPicture.hpp"
#include "2dfm/KgtPlayer.hpp"
#include "2dfm/KgtStage.hpp"
#include "engine/FileUtil.hpp"
#include "engine/MyString.hpp"
#include "engine/SoundClip.hpp"

#include <2dfm_reader.h>
#include <axmol.h>

#include <cstring>
#include <format>
#include <stdexcept>

namespace {

/// 打开 2DFM 文件流。调用方负责 closeFileStream
Kgt2dfmStream openFileStream(const std::string &path) {
    FILE *f = fopenUtf8(path, "rb");
    if (!f) {
        throw std::runtime_error("Failed to open 2dfm file: " + path);
    }
    return kgt_stream_from_file(f);
}

void closeFileStream(Kgt2dfmStream &stream) {
    if (stream.ctx) {
        std::fclose(static_cast<FILE *>(stream.ctx));
        stream.ctx = nullptr;
    }
}

/// 库里的 BGRA 结构与游戏侧布局一致，直接复用 createPalette
const _2dfm::ColorBgra *asColorBgra(const Kgt2dfmColorBgra palette[256]) {
    return reinterpret_cast<const _2dfm::ColorBgra *>(palette);
}

// ═══════════════════════════════════════════════════════════════════════════
// 公共资源：脚本 / 脚本项 / 共享调色盘 / 精灵帧 / 声音
// 四种文件完全复用这一段
// ═══════════════════════════════════════════════════════════════════════════
void fillCommonResource(CommonResource *cr,
                        const Kgt2dfmCommonResource &src,
                        std::string_view soundPathTemplate) {
    // ── 1. 脚本信息 ──
    cr->scripts.reserve(src.scriptCount);
    for (int i = 0; i < src.scriptCount; ++i) {
        KgtScript s;
        s.flags = static_cast<ScriptSpecialFlag>(src.scripts[i].flags);
        s.name = gbkToUtf8(src.scripts[i].name);
        s.startIdx = src.scripts[i].startIdx;
        s.endIdx = src.scripts[i].endIdx;
        cr->scripts.emplace_back(std::move(s));
    }

    // ── 2. 脚本项信息 ──
    cr->scriptItems.reserve(src.scriptItemCount);
    for (int i = 0; i < src.scriptItemCount; ++i) {
        auto *item = new _2dfm::ScriptItem();
        std::memcpy(item, &src.scriptItems[i], sizeof(_2dfm::ScriptItem));
        cr->scriptItems.emplace_back(item);
    }

    // ── 3. 共享调色盘 ──
    for (int i = 0; i < 8; ++i) {
        cr->sharedPalettes[i] = createPalette(asColorBgra(src.sharedPalettes[i]), false);
    }

    // ── 4. 精灵帧 ──
    cr->spriteFrames.reserve(src.pictureCount);
    for (int i = 0; i < src.pictureCount; ++i) {
        auto &sfi = cr->spriteFrames.emplace_back();
        // 构造一个临时 _2dfm::Picture 供 setFrom2dfmPicture 使用（借用指针，不拥有）
        _2dfm::Picture tempPic;
        std::memcpy(&tempPic.header, &src.pictures[i].header, sizeof(_2dfm::PictureHeader));
        tempPic.content = src.pictures[i].content;
        sfi.setFrom2dfmPicture(&tempPic);
        sfi.setSharedPalettes(cr->sharedPalettes);
    }

    // ── 5. 声音 ──
    cr->sounds.reserve(src.soundCount);
    for (int i = 0; i < src.soundCount; ++i) {
        const auto &snd = src.sounds[i];
        auto *copySound = new _2dfm::Sound();
        std::memcpy(&copySound->header, &snd.header, sizeof(_2dfm::SoundItemHeader));
        if (snd.contentSize > 0) {
            auto *content = static_cast<byte *>(std::malloc(snd.contentSize));
            std::memcpy(content, snd.content, snd.contentSize);
            copySound->content = content;
        } else {
            copySound->content = nullptr;
        }

        auto serialNo = cr->serialNo;
        auto idx = static_cast<size_t>(i);
        auto vfp = std::vformat(soundPathTemplate, std::make_format_args(serialNo, idx));
        cr->sounds.emplace_back(new SoundClip(copySound, std::move(vfp)));
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// 专有数据：四种文件各自分叉
// ═══════════════════════════════════════════════════════════════════════════

void fillGameSpecific(KgtGame &out, const Kgt2dfmGame &raw) {
    out.projectName = gbkToUtf8(raw.projectName);

    out.playerNames.reserve(_2dfm::maxPlayerNum);
    for (int i = 0; i < _2dfm::maxPlayerNum; ++i) {
        out.playerNames.emplace_back(gbkToUtf8(raw.playerNames[i]));
    }
    out.stageNames.reserve(_2dfm::maxStageNum);
    for (int i = 0; i < _2dfm::maxStageNum; ++i) {
        out.stageNames.emplace_back(gbkToUtf8(raw.stageNames[i]));
    }
    out.demoNames.reserve(_2dfm::maxDemoNum);
    for (int i = 0; i < _2dfm::maxDemoNum; ++i) {
        out.demoNames.emplace_back(gbkToUtf8(raw.demoNames[i]));
    }

    out.reactions.reserve(_2dfm::maxReactionNum);
    for (int i = 0; i < _2dfm::maxReactionNum; ++i) {
        Reaction r;
        r.name = gbkToUtf8(raw.reactions[i].name);
        r.isHurtAction = raw.reactions[i].isHurtAction != 0;
        out.reactions.emplace_back(std::move(r));
    }
    out.throwReactions.reserve(_2dfm::maxThrowReactionNum);
    for (int i = 0; i < _2dfm::maxThrowReactionNum; ++i) {
        out.throwReactions.emplace_back(gbkToUtf8(raw.throwReactions[i].name));
    }

    out.recoverTimeConfig.gap = raw.recoverTimeConfig.gap;
    out.recoverTimeConfig.attackRecoverTime = raw.recoverTimeConfig.attackRecoverTime;
    out.recoverTimeConfig.defenceRecoverTime = raw.recoverTimeConfig.defenceRecoverTime;
    out.recoverTimeConfig.clashRecoverTime = raw.recoverTimeConfig.clashRecoverTime;

    out.demoConfig.titleDemoId = raw.demoConfig.titleDemoId;
    out.demoConfig.storyModeCharSelectDemoId = raw.demoConfig.storyModeCharSelectDemoId;
    out.demoConfig.oneVsOneModeCharSelectDemoId = raw.demoConfig.oneVsOneModeCharSelectDemoId;
    out.demoConfig.teamModeCharSelectDemoId = raw.demoConfig.teamModeCharSelectDemoId;
    out.demoConfig.continueDemoId = raw.demoConfig.continueDemoId;
    out.demoConfig.openingDemoId = raw.demoConfig.openingDemoId;

    out.projectBaseConfig.encryptGame = raw.projectBaseConfig.encryptGame != 0;
    out.projectBaseConfig.allowClash = raw.projectBaseConfig.allowClash != 0;
    out.projectBaseConfig.enableStoryMode = raw.projectBaseConfig.enableStoryMode != 0;
    out.projectBaseConfig.enable1V1Mode = raw.projectBaseConfig.enable1V1Mode != 0;
    out.projectBaseConfig.enableTeamMode = raw.projectBaseConfig.enableTeamMode != 0;
    out.projectBaseConfig.showHpAfterHpBar = raw.projectBaseConfig.showHpAfterHpBar != 0;
    out.projectBaseConfig.pressToStart = raw.projectBaseConfig.pressToStart != 0;

    out.charSelectConfig.selectBoxStartPos =
        ax::Vec2(raw.charSelectConfig.selectBoxStartX, raw.charSelectConfig.selectBoxStartY);
    out.charSelectConfig.playerAvatarIconSize =
        ax::Vec2(raw.charSelectConfig.iconWidth, raw.charSelectConfig.iconHeight);
    out.charSelectConfig.rowCount = raw.charSelectConfig.rowNum;
    out.charSelectConfig.columnCount = raw.charSelectConfig.columnNum;
    out.charSelectConfig.player1PortraitPos =
        ax::Vec2(raw.charSelectConfig.player1PortraitX, raw.charSelectConfig.player1PortraitY);
    out.charSelectConfig.player1PortraitOffset =
        ax::Vec2(raw.charSelectConfig.player1PortraitTeamOffsetX, raw.charSelectConfig.player1PortraitTeamOffsetY);
    out.charSelectConfig.player2PortraitPos =
        ax::Vec2(raw.charSelectConfig.player2PortraitX, raw.charSelectConfig.player2PortraitY);
    out.charSelectConfig.player2PortraitOffset =
        ax::Vec2(raw.charSelectConfig.player2PortraitTeamOffsetX, raw.charSelectConfig.player2PortraitTeamOffsetY);

    for (int i = 0; i < _2dfm::maxPlayerNum; ++i) {
        out.playerSelectableInfos[i] = raw.playerSelectableInfos[i];
    }
}

void fillPlayerSpecific(KgtPlayer &out, const Kgt2dfmPlayer &raw) {
    out.playerName = gbkToUtf8(raw.playerName);

    // ── 1. 出招指令 ──
    out.commands.reserve(raw.commandCount);
    for (int i = 0; i < raw.commandCount; ++i) {
        const auto &rawCmd = raw.commands[i];
        PlayerCommand cmd;
        cmd.name = gbkToUtf8(rawCmd.name);
        cmd.timeWindow = rawCmd.timeWindow;
        cmd.airScriptId = rawCmd.airScriptId;
        cmd.standNearScriptId = rawCmd.standNearScriptId;
        cmd.standFarScriptId = rawCmd.standFarScriptId;
        cmd.crouchScriptId = rawCmd.crouchScriptId;

        // 指令步骤（最多 10 步，只有活动步骤保留）
        for (int s = 0; s < 10; ++s) {
            const int16_t data = rawCmd.cmdData[s];
            const auto state = static_cast<CommandStepState>((data >> 12) & 0x03);
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
        out.commands.push_back(std::move(cmd));
    }

    // ── 2. 中招反应绑定 ──
    if (raw.hurtBindCount > 0) {
        out.hurtBinds.assign(raw.hurtBinds, raw.hurtBinds + raw.hurtBindCount);
    }

    // ── 3. 投掷反应 ──
    out.throwActions.reserve(raw.throwActionCount);
    for (int i = 0; i < raw.throwActionCount; ++i) {
        const auto &rawThrow = raw.throwActions[i];
        out.throwActions.push_back(ThrowActionInfo{rawThrow.picNo, rawThrow.offsetX, rawThrow.offsetY});
    }

    // ── 4. AI 档位 ──
    out.aiSlots.reserve(KGT2DFM_AI_SLOT_COUNT);
    for (int i = 0; i < KGT2DFM_AI_SLOT_COUNT; ++i) {
        const auto &rawAi = raw.aiSlots[i];
        AiSlotInfo slot;
        slot.name = gbkToUtf8(rawAi.name);
        slot.airFlag = rawAi.aiAirFlag;
        slot.probability = rawAi.aiProbability;
        slot.rangeMin = rawAi.rangeMin;
        slot.rangeMax = rawAi.rangeMax;

        for (int c = 0; c < KGT2DFM_AI_SLOT_CMD_COUNT; ++c) {
            const auto &rawAiCmd = rawAi.aiCmds[c];
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
        out.aiSlots.push_back(std::move(slot));
    }

    // ── 5. 角色属性 ──
    out.stats.age = raw.age;
    out.stats.sexFlag = raw.sexFlag;
    out.stats.hpYPos = raw.statsConfig.hpYPos;
    out.stats.nearThreshold = raw.statsConfig.nearThreshold;
    out.stats.defenceAdjustRate = raw.statsConfig.defenceAdjustRate;
    out.stats.hpAdjustStartPos = raw.statsConfig.hpAdjustStartPos;
    out.stats.hpAdjustRate = raw.statsConfig.hpAdjustRate;
    out.stats.comboAdjustRate = raw.statsConfig.comboAdjustRate;
    out.stats.defenceKey = raw.statsConfig.defenceKey;
    out.stats.hp = raw.statsConfig.hp;
    out.stats.sp = raw.statsConfig.sp;
    out.stats.maxSpSlot = raw.statsConfig.maxSpSlot;
    out.stats.playerFlags = raw.statsConfig.playerFlags;
    out.stats.spAddWhenIAtk = raw.statsConfig.spAddWhenIAtk;
    out.stats.spAddWhenEnemyAtk = raw.statsConfig.spAddWhenEnemyAtk;
    out.stats.initSpSlot = raw.statsConfig.initSpSlot;

    // ── 6. 故事模式条目 ──
    out.storyItems.reserve(KGT2DFM_STORY_ITEM_COUNT);
    for (int i = 0; i < KGT2DFM_STORY_ITEM_COUNT; ++i) {
        const auto &rawStory = raw.storyItems[i];
        StoryItem item;
        item.type = static_cast<StoryItemType>(rawStory.itemType);

        switch (item.type) {
            case StoryItemType::BATTLE: {
                const uint8_t *d = rawStory.rawData;
                item.battleInfo.stageId = d[0];
                item.battleInfo.winRoundCount = d[1];
                item.battleInfo.hpRecoverRate = d[4];
                item.battleInfo.startRoundNum = d[5];
                item.battleInfo.roundTime = *reinterpret_cast<const int16_t *>(d + 6);
                item.battleInfo.playerStartXPos = *reinterpret_cast<const int16_t *>(d + 8);
                item.battleInfo.flags = *reinterpret_cast<const int32_t *>(d + 12);
                item.battleInfo.compareWithWhoWhenTimeout = d[16];
                item.battleInfo.winPointGetWhenTimeout = d[17];
                item.battleInfo.whoWillGotPointWhenKo = d[18];
                item.battleInfo.winPointGetWhenKo = d[19];

                const auto *cpuCfgs = reinterpret_cast<const Kgt2dfmStoryCpuConfig *>(d + 24);
                for (int c = 0; c < 7; ++c) {
                    const auto &src = cpuCfgs[c];
                    auto &dst = item.battleInfo.cpuConfigs[c];
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
            case StoryItemType::DEMO:
                item.demoInfo.demoId = rawStory.rawData[0];
                break;
            case StoryItemType::SWITCH: {
                const uint8_t *d = rawStory.rawData;
                item.switchInfo.conditionType = d[0];
                item.switchInfo.conditionValue = d[1];
                item.switchInfo.jumpOffset = d[4];
                break;
            }
            default:
                break;
        }
        out.storyItems.push_back(std::move(item));
    }
}

void fillDemoSpecific(KgtDemo &out, const Kgt2dfmDemo &raw) {
    out.demoName = gbkToUtf8(raw.demoName);
    out.config.bgmSoundId = raw.config.bgmSoundId;
    out.config.pressToSkip = raw.config.pressToSkip != 0;
    out.config.totalTime = raw.config.totalTime;
}

void fillStageSpecific(KgtStage &out, const Kgt2dfmStage &raw) {
    out.stageName = gbkToUtf8(raw.stageName);
    out.bgmSoundId = raw.config.bgmSoundId;
}

// ═══════════════════════════════════════════════════════════════════════════
// 唯一读取骨架
//   TRaw   : lib2dfm_reader 的原始结构
//   TResult: 游戏侧结构（KgtGame / KgtPlayer / KgtDemo / KgtStage）
// ═══════════════════════════════════════════════════════════════════════════
template <typename TRaw, typename TResult, typename TFillSpecific>
std::shared_ptr<TResult> loadResourceFile(const std::string &filepath,
                                          int serialNo,
                                          std::string_view soundPathTemplate,
                                          TRaw *(*readRaw)(Kgt2dfmStream *),
                                          void (*freeRaw)(TRaw *),
                                          TFillSpecific fillSpecific) {
    auto result = std::make_shared<TResult>();
    result->serialNo = serialNo;

    auto stream = openFileStream(filepath);
    TRaw *raw = readRaw(&stream);
    closeFileStream(stream);
    if (raw == nullptr) {
        throw std::runtime_error("Failed to parse 2dfm file: " + filepath);
    }

    fillCommonResource(result.get(), raw->common, soundPathTemplate);
    fillSpecific(*result, *raw);

    freeRaw(raw);
    return result;
}

} // namespace

namespace kgt {

std::shared_ptr<KgtGame> readGameFile(const std::string &filepath) {
    return loadResourceFile<Kgt2dfmGame, KgtGame>(
        filepath, 0, "kgt/{}/sound/{}.wav", &kgt_read_game, &kgt_free_game,
        [](KgtGame &out, const Kgt2dfmGame &raw) { fillGameSpecific(out, raw); });
}

std::shared_ptr<KgtPlayer> readPlayerFile(const std::string &filepath, int playerNo) {
    return loadResourceFile<Kgt2dfmPlayer, KgtPlayer>(
        filepath, playerNo, "player/{}/sound/{}.wav", &kgt_read_player, &kgt_free_player,
        [](KgtPlayer &out, const Kgt2dfmPlayer &raw) { fillPlayerSpecific(out, raw); });
}

std::shared_ptr<KgtDemo> readDemoFile(const std::string &filepath, int demoNo) {
    return loadResourceFile<Kgt2dfmDemo, KgtDemo>(
        filepath, demoNo, "demo/{}/sound/{}.wav", &kgt_read_demo, &kgt_free_demo,
        [](KgtDemo &out, const Kgt2dfmDemo &raw) { fillDemoSpecific(out, raw); });
}

std::shared_ptr<KgtStage> readStageFile(const std::string &filepath, int stageNo) {
    return loadResourceFile<Kgt2dfmStage, KgtStage>(
        filepath, stageNo, "stage/{}/sound/{}.wav", &kgt_read_stage, &kgt_free_stage,
        [](KgtStage &out, const Kgt2dfmStage &raw) { fillStageSpecific(out, raw); });
}

} // namespace kgt
