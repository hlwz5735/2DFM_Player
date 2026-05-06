//
// Created by limen on 2024/11/23.
//

#ifndef GAMEMANAGER_HPP
#define GAMEMANAGER_HPP

#include "engine/Singleton.hpp"

namespace ax::backend {
class ProgramState;
}

/**
 * GameManager - 游戏逻辑和配置管理器
 *
 * 注意：不再持有游戏资源（KgtGame/KgtStage 等）。
 * 资源管理已移交 ResourcePool。
 */
class GameManager : public Singleton<GameManager> {
public:
    bool init();

    uint64_t getCustomShaderProgramId() const;
};

#endif //GAMEMANAGER_HPP
