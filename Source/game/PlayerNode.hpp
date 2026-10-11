//
// Created by limen on 2025/1/10.
//

#ifndef PLAYERNODE_HPP
#define PLAYERNODE_HPP

#include <axmol.h>

#include "engine/KgtNode.hpp"

class PlayerNode : public KgtNode {
public:
    bool initWithVisibleHeight(int visibleHeight = 640) override;
    void update(float delta) override;
    void destroy() override;

    /// 添加物体Node
    void addObjectNode(PlayerNode *node, int manageNo);
    void removeObjectNode(PlayerNode *node);

    bool getIsObjectNode() const { return isObjectNode; }
    void setIsObjectNode(bool val) { isObjectNode = val; }

    /// “物”的归属者（生成它的角色节点）：回收时要先把自己从它的管理表里摘掉
    void setObjectManager(PlayerNode *node) { objectManager = node; }
    PlayerNode *getObjectManager() const { return objectManager; }

    /**
     * 标记回收（“物”脚本结束后调用）。
     *
     * 只做标记，真正的销毁留到本节点自己的下一次 update：
     * 脚本结束是在组件更新过程中发生的，此刻把节点从父节点摘掉并不安全；
     * 等下一帧进到自己的 update 时再摘，那时没有任何遍历正压在这个节点上。
     * 标记之前调用方应已把精灵隐藏掉（“物”结束就该看不见了）。
     */
    void markForRecycle() { pendingRecycle = true; }
    bool isPendingRecycle() const { return pendingRecycle; }

    /**
     * 视口（虚拟摄像机）。角色的屏幕位置 = 逻辑坐标 - 摄像机偏移，
     * 这样角色才会和同样受摄像机影响的背景层一起滚动（见 StageCameraNode）。
     * 不设置时按偏移 0 处理。
     */
    void setViewCamera(class StageCameraNode *camera) { viewCamera = camera; }
    class StageCameraNode *getViewCamera() const { return viewCamera; }

    void destroyManagedObjects();
private:
    /// 受管理的已生成的物体节点，前9个是固定受管理的，之后则为无条件出现的
    std::vector<PlayerNode *> managedObjects;
    bool isObjectNode = false;
    /// 见 markForRecycle()
    bool pendingRecycle = false;
    /// 生成自己的角色节点（仅“物”有）
    PlayerNode *objectManager = nullptr;
    class StageCameraNode *viewCamera = nullptr;
};

#endif //PLAYERNODE_HPP
