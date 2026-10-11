//
// Created by limen on 2026/5/6.
//

#include "ResourcePool.hpp"
#include "GameConfig.hpp"
#include "ResourceLoader.hpp"

#include "2dfm/KgtDemo.hpp"
#include "2dfm/KgtGame.hpp"
#include "2dfm/KgtPlayer.hpp"
#include "2dfm/KgtStage.hpp"

#include <axmol.h>

#include <format>
#include <stdexcept>

// ─── 公共接口实现 ─────────────────────────────────────────────────────────────
// 说明：所有格式解析都集中在 ResourceLoader（唯一读取路径），
//       这里只负责路径解析、缓存与生命周期管理。

std::shared_ptr<KgtGame> ResourcePool::loadKgtGame(const std::string& filepath) {
    // 同一路径命中缓存直接返回；换路径则旧游戏整体失效，避免用到上一个游戏的名称表/资源
    if (cachedKgtGame && cachedKgtPath == filepath) return cachedKgtGame;
    if (cachedKgtGame) {
        AXLOGI("[ResourcePool] switching KGT: '{}' -> '{}'", cachedKgtPath, filepath);
        // 旧资源延后到后续渲染帧再释放：避免旧纹理删除与新纹理创建在同一帧内交织
        pendingReleaseKgt = std::move(cachedKgtGame);
        pendingReleasePlayers = std::move(cachedPlayers);
        pendingReleaseDemo = std::move(cachedDemo);
        pendingReleaseStage = std::move(cachedStage);
        cachedDemoNo = -1;
        cachedStageNo = -1;
        schedulePendingRelease();
    }

    std::shared_ptr<KgtGame> result;
    try {
        result = kgt::readGameFile(filepath);
    } catch (const std::exception& e) {
        AXLOGE("[ResourcePool] loadKgtGame failed for '{}': {}", filepath, e.what());
        return nullptr;
    }

    cachedKgtGame = result;
    cachedKgtPath = filepath;
    return cachedKgtGame;
}

std::shared_ptr<KgtGame> ResourcePool::getKgtGame() const {
    return cachedKgtGame;
}

std::shared_ptr<KgtPlayer> ResourcePool::loadPlayer(int playerNo) {
    auto it = cachedPlayers.find(playerNo);
    if (it != cachedPlayers.end()) return it->second;

    auto kgt = getKgtGame();
    if (!kgt) {
        AXLOGE("[ResourcePool] loadPlayer({}) failed: kgt is null", playerNo);
        return nullptr;
    }
    if (playerNo < 0 || playerNo >= (int)kgt->playerNames.size()) {
        AXLOGE("[ResourcePool] loadPlayer failed: playerNo={} out of range [0, {})", playerNo, kgt->playerNames.size());
        return nullptr;
    }

    auto& playerName = kgt->playerNames[playerNo];
    if (playerName.empty()) {
        AXLOGE("[ResourcePool] loadPlayer({}) failed: playerName is empty", playerNo);
        return nullptr;
    }

    auto& gameConfig = GameConfig::getInstance();
    auto fullPath = std::format("{}/{}.player", gameConfig.getGameBasePath(), playerName);

    std::shared_ptr<KgtPlayer> result;
    try {
        result = kgt::readPlayerFile(fullPath, playerNo);
    } catch (const std::exception& e) {
        AXLOGE("[ResourcePool] loadPlayer({}) failed for '{}': {}", playerNo, fullPath, e.what());
        return nullptr;
    }

    cachedPlayers[playerNo] = result;
    return result;
}

std::shared_ptr<KgtPlayer> ResourcePool::getPlayer(int playerNo) const {
    auto it = cachedPlayers.find(playerNo);
    return it != cachedPlayers.end() ? it->second : nullptr;
}

std::shared_ptr<KgtDemo> ResourcePool::loadDemo(int demoNo) {
    if (cachedDemo && cachedDemoNo == demoNo) return cachedDemo;

    // 加载新 Demo，旧的由 shared_ptr 自动管理生命周期
    // 只要还有持有者（如 GameDemo、Interceptor），旧资源就不会释放
    cachedDemo.reset();
    cachedDemoNo = -1;

    auto kgt = getKgtGame();
    if (!kgt) {
        AXLOGE("[ResourcePool] loadDemo({}) failed: kgt is null", demoNo);
        return nullptr;
    }
    if (demoNo < 0 || demoNo >= (int)kgt->demoNames.size()) {
        AXLOGE("[ResourcePool] loadDemo failed: demoNo={} out of range [0, {})", demoNo, kgt->demoNames.size());
        return nullptr;
    }

    auto& demoName = kgt->demoNames[demoNo];
    if (demoName.empty()) {
        AXLOGE("[ResourcePool] loadDemo({}) failed: demoName is empty", demoNo);
        return nullptr;
    }

    auto& gameConfig = GameConfig::getInstance();
    auto fullPath = std::format("{}/{}.demo", gameConfig.getGameBasePath(), demoName);

    std::shared_ptr<KgtDemo> result;
    try {
        result = kgt::readDemoFile(fullPath, demoNo);
    } catch (const std::exception& e) {
        AXLOGE("[ResourcePool] loadDemo({}) failed for '{}': {}", demoNo, fullPath, e.what());
        return nullptr;
    }

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

    // 加载新 Stage，旧的由 shared_ptr 自动管理生命周期
    cachedStage.reset();
    cachedStageNo = -1;

    auto kgt = getKgtGame();
    if (!kgt) {
        AXLOGE("[ResourcePool] loadStage({}) failed: kgt is null", stageNo);
        return nullptr;
    }
    if (stageNo < 0 || stageNo >= (int)kgt->stageNames.size()) {
        AXLOGE("[ResourcePool] loadStage failed: stageNo={} out of range [0, {})", stageNo, kgt->stageNames.size());
        return nullptr;
    }

    auto& stageName = kgt->stageNames[stageNo];
    if (stageName.empty()) {
        AXLOGE("[ResourcePool] loadStage({}) failed: stageName is empty", stageNo);
        return nullptr;
    }

    auto& gameConfig = GameConfig::getInstance();
    auto fullPath = std::format("{}/{}.stage", gameConfig.getGameBasePath(), stageName);

    std::shared_ptr<KgtStage> result;
    try {
        result = kgt::readStageFile(fullPath, stageNo);
    } catch (const std::exception& e) {
        AXLOGE("[ResourcePool] loadStage({}) failed for '{}': {}", stageNo, fullPath, e.what());
        return nullptr;
    }

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

void ResourcePool::schedulePendingRelease() {
    auto* director = ax::Director::getInstance();
    if (director == nullptr) {
        // 极端情况：无 Director（理论不会发生），直接同步释放
        pendingReleaseKgt.reset();
        pendingReleasePlayers.clear();
        pendingReleaseDemo.reset();
        pendingReleaseStage.reset();
        return;
    }
    auto* scheduler = director->getScheduler();
    // repeat = 0 → 只执行一次（release + 1 次调度）
    scheduler->schedule(
        [this](float) {
            pendingReleaseKgt.reset();
            pendingReleasePlayers.clear();
            pendingReleaseDemo.reset();
            pendingReleaseStage.reset();
        },
        this, 0.f, 0, 0.f, false, "2dfm_pending_resource_release");
}

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

void ResourcePool::unloadAll() {
    if (auto* director = ax::Director::getInstance()) {
        director->getScheduler()->unschedule("2dfm_pending_resource_release", this);
    }
    cachedKgtGame.reset();
    cachedPlayers.clear();
    purgeDemo();
    purgeStage();
    cachedKgtPath.clear();
    pendingReleaseKgt.reset();
    pendingReleasePlayers.clear();
    pendingReleaseDemo.reset();
    pendingReleaseStage.reset();
}
