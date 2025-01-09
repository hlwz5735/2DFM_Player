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
};

#endif //KGTPLAYER_HPP
