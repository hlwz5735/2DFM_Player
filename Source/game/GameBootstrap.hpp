//
// Created by limen on 2026/9/9.
// GameBootstrap: 全局一次性初始化
//

#pragma once

#include "engine/Singleton.hpp"

/**
 * GameBootstrap - 全进程只执行一次的初始化入口
 *
 * 严格区分初始化边界：
 * - 这里只做“与具体 KGT 无关、进程内只需一次”的全局初始化：
 *   Input 输入系统、自定义着色器注册、GameConfig 磁盘初始读取。
 * - KGT 资源加载（ResourcePool）与场景创建不属于这里，
 *   由 MainScene::loadKgtFromConfig / 各场景按需触发。
 */
class GameBootstrap : public Singleton<GameBootstrap> {
public:
    /// 执行全局一次性初始化；重复调用为无操作（幂等）
    void initialize();

    bool isInitialized() const { return initialized; }

private:
    bool initialized = false;
};
