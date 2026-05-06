//
// Created by limen on 2026/5/6.
// ResourcePool: 游戏资源的池化管理
//

#pragma once

#include "engine/Singleton.hpp"
#include <memory>
#include <unordered_map>
#include <string>

struct KgtGame;
struct KgtPlayer;
struct KgtDemo;
struct KgtStage;
struct CommonResource;

/**
 * ResourcePool - 游戏资源池化管理器
 *
 * 职责：
 * 1. 通过 lib2dfm_reader 加载并缓存游戏资源
 * 2. 管理资源生命周期 (KGT 全局、Player 多个共存、Demo/Stage 单个活跃)
 * 3. 为 KgtFileUtil 提供虚拟路径查询接口
 */
class ResourcePool : public Singleton<ResourcePool> {
public:
    // ─── 资源加载/获取 (缓存命中则直接返回) ───

    /** 加载 KGT 主文件（全局唯一，整个游戏生命周期有效） */
    std::shared_ptr<KgtGame> loadKgtGame(const std::string& filepath);

    /** 获取已加载的 KGT (未加载返回 nullptr) */
    std::shared_ptr<KgtGame> getKgtGame() const;

    /** 加载 Player 文件。可同时缓存多个 */
    std::shared_ptr<KgtPlayer> loadPlayer(int playerNo);

    /** 获取已加载的 Player (未加载返回 nullptr) */
    std::shared_ptr<KgtPlayer> getPlayer(int playerNo) const;

    /** 加载 Demo 文件。同时只缓存一个活跃 Demo */
    std::shared_ptr<KgtDemo> loadDemo(int demoNo);

    /** 获取已加载的 Demo (未加载返回 nullptr) */
    std::shared_ptr<KgtDemo> getDemo(int demoNo) const;

    /** 加载 Stage 文件。同时只缓存一个活跃 Stage */
    std::shared_ptr<KgtStage> loadStage(int stageNo);

    /** 获取已加载的 Stage (未加载返回 nullptr) */
    std::shared_ptr<KgtStage> getStage(int stageNo) const;

    // ─── 虚拟路径查询 (供 KgtFileUtil 使用) ───

    enum class FileType { KGT, PLAYER, DEMO, STAGE };

    /**
     * 根据文件类型和编号查找对应的 CommonResource。
     * 如果资源未加载，返回 nullptr。
     */
    CommonResource* findResource(FileType type, int fileIndex) const;

    // ─── 资源释放 ───

    void purgePlayer(int playerNo);
    void purgeAllPlayers();
    void purgeDemo();
    void purgeStage();
    void purgeAll();

private:
    std::shared_ptr<KgtGame> cachedKgtGame;
    std::unordered_map<int, std::shared_ptr<KgtPlayer>> cachedPlayers;
    std::shared_ptr<KgtDemo> cachedDemo;
    int cachedDemoNo = -1;
    std::shared_ptr<KgtStage> cachedStage;
    int cachedStageNo = -1;
};
