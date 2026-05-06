//
// Created by limen on 2026/5/6.
//

#include "ResourcePool.hpp"
#include "GameConfig.hpp"

#include "2dfm/KgtGame.hpp"
#include "2dfm/KgtPlayer.hpp"
#include "2dfm/KgtDemo.hpp"
#include "2dfm/KgtStage.hpp"
#include "2dfm/2dfmFileReader.hpp"
#include "engine/MyString.hpp"
#include "engine/SoundClip.hpp"

#include <axmol.h>
#include <2dfm_reader.h>
#include <cstdio>
#include <format>
#include <stdexcept>

namespace {

// ─── 从库的 C 结构转换为主工程的 C++ 结构 ───

KgtPalette* convertPalette(const Kgt2dfmColorBgra palette[256], bool isPrivate) {
    auto* result = new KgtPalette();
    for (int i = 0; i < 256; ++i) {
        _2dfm::ColorBgra color;
        color.value = palette[i].value;
        result->colors[i] = color;

        if (color.channel.red == 0 && color.channel.green == 0 && color.channel.blue == 0) {
            result->colors[i].channel.alpha = 0;
        } else {
            if (isPrivate) {
                result->colors[i].channel.alpha = 255;
            } else {
                result->colors[i].channel.alpha = color.channel.alpha == 1 ? 255 : 0;
            }
        }
    }
    return result;
}

void fillCommonResource(CommonResource* cr, const Kgt2dfmCommonResource& src, const std::string& fmtTemplate) {
    cr->scripts.reserve(src.scriptCount);
    for (int i = 0; i < src.scriptCount; ++i) {
        KgtScript s;
        s.flags = static_cast<ScriptSpecialFlag>(src.scripts[i].flags);
        s.name = gbkToUtf8(src.scripts[i].name);
        s.startIdx = src.scripts[i].startIdx;
        s.endIdx = src.scripts[i].endIdx;
        cr->scripts.emplace_back(std::move(s));
    }

    cr->scriptItems.reserve(src.scriptItemCount);
    for (int i = 0; i < src.scriptItemCount; ++i) {
        auto* item = new _2dfm::ScriptItem();
        std::memcpy(item, &src.scriptItems[i], sizeof(_2dfm::ScriptItem));
        cr->scriptItems.emplace_back(item);
    }

    // 共享调色盘
    for (int i = 0; i < 8; ++i) {
        cr->sharedPalettes[i] = convertPalette(src.sharedPalettes[i], false);
    }

    // 精灵帧
    cr->spriteFrames.reserve(src.pictureCount);
    for (int i = 0; i < src.pictureCount; ++i) {
        auto& sfi = cr->spriteFrames.emplace_back();
        // 构造一个临时 _2dfm::Picture 供 setFrom2dfmPicture 使用
        _2dfm::Picture tempPic;
        std::memcpy(&tempPic.header, &src.pictures[i].header, sizeof(_2dfm::PictureHeader));
        tempPic.content = src.pictures[i].content; // 借用指针，不拥有
        sfi.setFrom2dfmPicture(&tempPic);
        sfi.setSharedPalettes(cr->sharedPalettes);
    }

    // 声音
    cr->sounds.reserve(src.soundCount);
    for (int i = 0; i < src.soundCount; ++i) {
        auto& snd = src.sounds[i];
        auto* copySound = new _2dfm::Sound();
        std::memcpy(&copySound->header, &snd.header, sizeof(_2dfm::SoundItemHeader));
        if (snd.contentSize > 0) {
            auto* content = static_cast<byte*>(std::malloc(snd.contentSize));
            std::memcpy(content, snd.content, snd.contentSize);
            copySound->content = content;
        } else {
            copySound->content = nullptr;
        }

        auto serialNo = cr->serialNo;
        auto idx = static_cast<size_t>(i);
        auto vfp = std::vformat(fmtTemplate, std::make_format_args(serialNo, idx));
        cr->sounds.emplace_back(new SoundClip(copySound, std::move(vfp)));
    }
}

Kgt2dfmStream openFileStream(const std::string& path) {
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    return kgt_stream_from_file(f);
}

void closeFileStream(Kgt2dfmStream& stream) {
    if (stream.ctx) {
        std::fclose(static_cast<FILE*>(stream.ctx));
        stream.ctx = nullptr;
    }
}

} // anonymous namespace

// ─── 公共接口实现 ─────────────────────────────────────────────────────────────

std::shared_ptr<KgtGame> ResourcePool::loadKgtGame(const std::string& filepath) {
    if (cachedKgtGame) return cachedKgtGame;

    Kgt2dfmStream stream;
    try {
        stream = openFileStream(filepath);
    } catch (...) {
        return nullptr;
    }

    auto* raw = kgt_read_game(&stream);
    closeFileStream(stream);
    if (!raw) return nullptr;

    auto result = std::make_shared<KgtGame>();
    result->serialNo = 0;
    result->projectName = gbkToUtf8(raw->projectName);

    fillCommonResource(result.get(), raw->common, "kgt/{}/sound/{}.wav");

    // 角色名称
    result->playerNames.reserve(50);
    for (int i = 0; i < 50; ++i) {
        result->playerNames.emplace_back(gbkToUtf8(raw->playerNames[i]));
    }
    // 场景名称
    result->stageNames.reserve(50);
    for (int i = 0; i < 50; ++i) {
        result->stageNames.emplace_back(gbkToUtf8(raw->stageNames[i]));
    }
    // DEMO 名称
    result->demoNames.reserve(100);
    for (int i = 0; i < 100; ++i) {
        result->demoNames.emplace_back(gbkToUtf8(raw->demoNames[i]));
    }
    // 受击反应
    result->reactions.reserve(200);
    for (int i = 0; i < 200; ++i) {
        Reaction r;
        r.name = gbkToUtf8(raw->reactions[i].name);
        r.isHurtAction = static_cast<bool>(raw->reactions[i].isHurtAction);
        result->reactions.emplace_back(std::move(r));
    }
    // 投掷反应
    result->throwReactions.reserve(200);
    for (int i = 0; i < 200; ++i) {
        result->throwReactions.emplace_back(gbkToUtf8(raw->throwReactions[i].name));
    }

    // 配置
    result->recoverTimeConfig.gap = raw->recoverTimeConfig.gap;
    result->recoverTimeConfig.attackRecoverTime = raw->recoverTimeConfig.attackRecoverTime;
    result->recoverTimeConfig.defenceRecoverTime = raw->recoverTimeConfig.defenceRecoverTime;
    result->recoverTimeConfig.clashRecoverTime = raw->recoverTimeConfig.clashRecoverTime;

    result->demoConfig.titleDemoId = raw->demoConfig.titleDemoId;
    result->demoConfig.storyModeCharSelectDemoId = raw->demoConfig.storyModeCharSelectDemoId;
    result->demoConfig.oneVsOneModeCharSelectDemoId = raw->demoConfig.oneVsOneModeCharSelectDemoId;
    result->demoConfig.teamModeCharSelectDemoId = raw->demoConfig.teamModeCharSelectDemoId;
    result->demoConfig.continueDemoId = raw->demoConfig.continueDemoId;
    result->demoConfig.openingDemoId = raw->demoConfig.openingDemoId;

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

    // 角色可选择性
    for (int i = 0; i < 50; ++i) {
        result->playerSelectableInfos[i] = raw->playerSelectableInfos[i];
    }

    kgt_free_game(raw);
    cachedKgtGame = result;
    return cachedKgtGame;
}

std::shared_ptr<KgtGame> ResourcePool::getKgtGame() const {
    return cachedKgtGame;
}

std::shared_ptr<KgtPlayer> ResourcePool::loadPlayer(int playerNo) {
    auto it = cachedPlayers.find(playerNo);
    if (it != cachedPlayers.end()) return it->second;

    auto kgt = getKgtGame();
    if (!kgt || playerNo < 0 || playerNo >= (int)kgt->playerNames.size()) return nullptr;

    auto& playerName = kgt->playerNames[playerNo];
    if (playerName.empty()) return nullptr;

    auto& gameConfig = GameConfig::getInstance();
    auto fullPath = std::format("{}/{}.player", gameConfig.getGameBasePath(), playerName);

    Kgt2dfmStream stream;
    try {
        stream = openFileStream(fullPath);
    } catch (...) {
        return nullptr;
    }

    auto* raw = kgt_read_player(&stream);
    closeFileStream(stream);
    if (!raw) return nullptr;

    auto result = std::make_shared<KgtPlayer>();
    result->serialNo = playerNo;
    result->playerName = gbkToUtf8(raw->playerName);
    fillCommonResource(result.get(), raw->common, "player/{}/sound/{}.wav");

    kgt_free_player(raw);
    cachedPlayers[playerNo] = result;
    return result;
}

std::shared_ptr<KgtPlayer> ResourcePool::getPlayer(int playerNo) const {
    auto it = cachedPlayers.find(playerNo);
    return it != cachedPlayers.end() ? it->second : nullptr;
}

std::shared_ptr<KgtDemo> ResourcePool::loadDemo(int demoNo) {
    if (cachedDemo && cachedDemoNo == demoNo) return cachedDemo;

    // 切换 Demo 时释放旧的
    purgeDemo();

    auto kgt = getKgtGame();
    if (!kgt || demoNo < 0 || demoNo >= (int)kgt->demoNames.size()) return nullptr;

    auto& demoName = kgt->demoNames[demoNo];
    if (demoName.empty()) return nullptr;

    auto& gameConfig = GameConfig::getInstance();
    auto fullPath = std::format("{}/{}.demo", gameConfig.getGameBasePath(), demoName);

    Kgt2dfmStream stream;
    try {
        stream = openFileStream(fullPath);
    } catch (...) {
        return nullptr;
    }

    auto* raw = kgt_read_demo(&stream);
    closeFileStream(stream);
    if (!raw) return nullptr;

    auto result = std::make_shared<KgtDemo>();
    result->serialNo = demoNo;
    result->demoName = gbkToUtf8(raw->demoName);
    fillCommonResource(result.get(), raw->common, "demo/{}/sound/{}.wav");
    result->config.bgmSoundId = raw->config.bgmSoundId;
    result->config.pressToSkip = raw->config.pressToSkip;
    result->config.totalTime = raw->config.totalTime;

    kgt_free_demo(raw);
    cachedDemo = result;
    cachedDemoNo = demoNo;
    return cachedDemo;
}

std::shared_ptr<KgtDemo> ResourcePool::getDemo(int demoNo) const {
    if (cachedDemo && cachedDemoNo == demoNo) return cachedDemo;
    return nullptr;
}

std::shared_ptr<KgtStage> ResourcePool::loadStage(int stageNo) {
    if (cachedStage && cachedStageNo == stageNo) return cachedStage;

    // 切换 Stage 时释放旧的
    purgeStage();

    auto kgt = getKgtGame();
    if (!kgt || stageNo < 0 || stageNo >= (int)kgt->stageNames.size()) return nullptr;

    auto& stageName = kgt->stageNames[stageNo];
    if (stageName.empty()) return nullptr;

    auto& gameConfig = GameConfig::getInstance();
    auto fullPath = std::format("{}/{}.stage", gameConfig.getGameBasePath(), stageName);

    Kgt2dfmStream stream;
    try {
        stream = openFileStream(fullPath);
    } catch (...) {
        return nullptr;
    }

    auto* raw = kgt_read_stage(&stream);
    closeFileStream(stream);
    if (!raw) return nullptr;

    auto result = std::make_shared<KgtStage>();
    result->serialNo = stageNo;
    result->stageName = gbkToUtf8(raw->stageName);
    fillCommonResource(result.get(), raw->common, "stage/{}/sound/{}.wav");
    result->bgmSoundId = raw->config.bgmSoundId;

    kgt_free_stage(raw);
    cachedStage = result;
    cachedStageNo = stageNo;
    return cachedStage;
}

std::shared_ptr<KgtStage> ResourcePool::getStage(int stageNo) const {
    if (cachedStage && cachedStageNo == stageNo) return cachedStage;
    return nullptr;
}

// ─── 虚拟路径查询 ─────────────────────────────────────────────────────────────

CommonResource* ResourcePool::findResource(FileType type, int fileIndex) const {
    switch (type) {
    case FileType::KGT:
        return cachedKgtGame.get();
    case FileType::PLAYER: {
        auto it = cachedPlayers.find(fileIndex);
        return it != cachedPlayers.end() ? it->second.get() : nullptr;
    }
    case FileType::DEMO:
        return (cachedDemo && cachedDemoNo == fileIndex) ? cachedDemo.get() : nullptr;
    case FileType::STAGE:
        return (cachedStage && cachedStageNo == fileIndex) ? cachedStage.get() : nullptr;
    }
    return nullptr;
}

// ─── 资源释放 ─────────────────────────────────────────────────────────────────

void ResourcePool::purgePlayer(int playerNo) {
    cachedPlayers.erase(playerNo);
}

void ResourcePool::purgeAllPlayers() {
    cachedPlayers.clear();
}

void ResourcePool::purgeDemo() {
    cachedDemo.reset();
    cachedDemoNo = -1;
}

void ResourcePool::purgeStage() {
    cachedStage.reset();
    cachedStageNo = -1;
}

void ResourcePool::purgeAll() {
    cachedKgtGame.reset();
    purgeAllPlayers();
    purgeDemo();
    purgeStage();
}
