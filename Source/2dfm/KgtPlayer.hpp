//
// Created by limen on 2025/1/9.
//

#ifndef KGTPLAYER_HPP
#define KGTPLAYER_HPP

#include "CommonResource.hpp"

/// 中招反应绑定信息
struct HurtBindInfo {
    /// 中招反应的ID
    int hurdId;
    /// 中招脚本的ID（编号）
    int scriptId;
    /// 光效物件的ID（脚本编号）
    int effectObjectId;
};
/// 投掷动作信息
struct ThrowActionInfo {
    /// 投掷动作的ID（和KGT对应）
    int throwActionId;
    /// 图号
    int picNo;
    /// X偏移量
    int offsetX;
    /// Y偏移量
    int offsetY;
};

struct KgtPlayer : CommonResource {
    std::string playerName;

    /// TODO: 受伤动作绑定信息
    std::vector<HurtBindInfo> hurtBinds;
    /// TODO: 投掷动作绑定信息
    std::vector<ThrowActionInfo> throwActions;
    /// TODO: 出招指令信息
    std::vector<void *> commandListEntries;
    /// TODO: AI出招条目
    std::vector<void *> aiEntries;
    /// TODO: 故事条目信息
    std::vector<void *> storyEntries;

    /// 通常站立 脚本项ID
    int standScriptId = 0;
    /// 前行 脚本项ID
    int forwardScriptId = 0;
    /// 后退 脚本项ID
    int backwardScriptId = 0;
    /// 上跳 脚本项ID
    int jumpUpScriptId = 0;
    /// 前跳 脚本项ID
    int jumpForwardScriptId = 0;
    /// 后跳 脚本项ID
    int jumpBackwardScriptId = 0;
    /// 空中落下 脚本项ID
    int fallingScriptId = 0;
    /// 下蹲 脚本项ID
    int crouchScriptId = 0;
    /// 蹲着 脚本项ID
    int crouchingScriptId = 0;
    /// 从下蹲站起 脚本项ID
    int standUpScriptId = 0;
    /// 蹲着前行 脚本项ID
    int crouchMoveForwardScriptId = 0;
    /// 蹲着后退 脚本项ID
    int crouchMoveBackwardScriptId = 0;
    /// 站立转身 脚本项ID
    int turnAroundScriptId = 0;
    /// 蹲着转身 脚本项ID
    int crouchTurnAroundScriptId = 0;
    /// 按键站立防御 脚本项ID
    int buttonBlockScriptId = 0;
    /// 按键蹲着防御 脚本项ID
    int buttonCrouchBlockScriptId = 0;
    /// 按键空中防御 脚本项ID
    int buttonAirBlockScriptId = 0;
    /// 开始时 脚本项ID
    int startActionScriptId = 0;
    /// 胜利 脚本项ID
    int victoryScriptId = 0;
    /// 失败 脚本项ID
    int failScriptId = 0;
    /// 平局 脚本项ID
    int drawGameScriptId = 0;
    /// 角色选择段落 脚本项ID
    int portraitScriptId = 0;
    /// 场景段落 脚本项ID
    int stageAvatarScriptId = 0;
    /// 影 脚本项ID
    int shadowScriptId = 0;


    /// 初始化角色基础脚本
    void initBasicScriptInfos();
};

#endif //KGTPLAYER_HPP
