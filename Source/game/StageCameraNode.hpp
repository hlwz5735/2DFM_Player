//
// Created by limen on 2024/11/25.
//

#ifndef STAGECAMERANODE_HPP
#define STAGECAMERANODE_HPP

#include <axmol.h>

class KgtNode;

/**
 * 虚拟摄像机。
 *
 * 它本身不移动任何子节点，而是作为一个“视口偏移量”被各节点读取：
 * 背景层通过 ParallaxComponent 按视差比例取用（StageNode），
 * 角色通过 PlayerNode::setViewCamera 取用。
 *
 * 摄像机位置始终夹在场景范围内（[0, 场景尺寸 - 视口尺寸]），因此不会滚出场景。
 */
class StageCameraNode : public ax::Node {
public:
    bool init() override;
    void update(float delta) override;

    ax::Vec2 getOffset() const { return _position - lastPosition; }
    ax::Size getViewSize() const { return _director->getVisibleSize(); }

    void setPosition(float x, float y) override;
    void setPosition(const ax::Vec2 &pos) override { StageCameraNode::setPosition(pos.x, pos.y); }

    /// 跟随目标：每帧把摄像机移到“让目标尽量居中”的位置，并夹在场景范围内
    void setFollowTarget(KgtNode *target) { followTarget = target; }
    KgtNode *getFollowTarget() const { return followTarget; }

    /**
     * 是否连垂直方向一起跟随。默认关闭。
     * 2DFM 的对战镜头垂直方向是固定的：跟随会让每一次跳跃都拖着整个场景上下滚。
     */
    void setFollowVertical(bool value) { followVertical = value; }
    bool getFollowVertical() const { return followVertical; }

    void reset();

private:
    /// 按跟随目标算出摄像机应有的位置（返回值已夹在场景范围内）
    ax::Vec2 calcFollowPosition() const;

    KgtNode *followTarget = nullptr;
    bool followVertical = false;

    ax::Vec2 lastPosition;
};

#endif //STAGECAMERANODE_HPP
