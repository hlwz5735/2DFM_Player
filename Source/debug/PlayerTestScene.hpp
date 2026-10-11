//
// Created by Qoder on 2026/5/10.
//

#ifndef PLAYERTESTSCENE_HPP
#define PLAYERTESTSCENE_HPP

#include <axmol.h>
#include <memory>
#include <string>

struct KgtPlayer;
class GameStage;
class PlayerNode;
class CharacterController;

/**
 * 角色测试场景。
 *
 * 角色的状态机在 CharacterController 里，这个场景只负责：搭场景、造角色、把控制器挂上去、
 * 把摄像机对准角色，并把当前状态显示出来。输入由控制器自己读取。
 */
class PlayerTestScene final : public ax::Scene {
public:
    bool init() override;
    void update(float delta) override;
    void onExit() override;

private:
    void loadStageAndPlayer();
    /// 左上角显示两轴状态，方便肉眼看状态机
    void refreshStateLabel();

    GameStage *gameStage = nullptr;
    PlayerNode *playerNode = nullptr;
    CharacterController *controller = nullptr;
    ax::Label *stateLabel = nullptr;
    std::shared_ptr<KgtPlayer> playerData;

    /// 地面Y坐标
    float groundY = 0.f;

    /// 上一次刷到标签上的状态文本（避免每帧重建文字纹理）
    std::string lastStateText;
};

#endif //PLAYERTESTSCENE_HPP
