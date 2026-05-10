//
// Created by Qoder on 2026/5/10.
//

#ifndef PLAYERTESTSCENE_HPP
#define PLAYERTESTSCENE_HPP

#include <axmol.h>
#include <memory>

struct KgtPlayer;
class GameStage;
class PlayerNode;

/// 角色状态枚举
enum class PlayerState {
    STAND,          // 站立待机
    WALK_FORWARD,   // 前行
    WALK_BACKWARD,  // 后退
    JUMP_UP,        // 上跳
    JUMP_FORWARD,   // 前跳
    JUMP_BACKWARD,  // 后跳
    FALLING,        // 下落中
    CROUCH,         // 下蹲过渡
    CROUCHING,      // 蹲着
    STAND_UP,       // 站起
};

class PlayerTestScene final : public ax::Scene {
public:
    bool init() override;
    void update(float delta) override;
    void onExit() override;

private:
    void loadStageAndPlayer();
    void switchPlayerScript(int scriptId);
    void handleInput();
    void updatePlayerState();

    GameStage *gameStage = nullptr;
    PlayerNode *playerNode = nullptr;
    std::shared_ptr<KgtPlayer> playerData;

    PlayerState currentState = PlayerState::STAND;
    PlayerState prevState = PlayerState::STAND;

    /// 地面Y坐标
    float groundY = 0.f;
};

#endif //PLAYERTESTSCENE_HPP
