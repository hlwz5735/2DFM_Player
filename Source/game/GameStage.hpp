//
// Created by limen on 2025/1/9.
//

#ifndef GAMESTAGE_HPP
#define GAMESTAGE_HPP

#include <axmol.h>
#include <memory>

struct KgtStage;

class GameStage : public ax::Node {
public:
    bool init() override;
    void update(float delta) override;
    void onExit() override;

    void load(int stageNo);
    void unload();

    /// 场景共用的虚拟摄像机（角色与背景层都从它取视口偏移）
    class StageCameraNode *getCameraNode() const { return cameraNode; }

    /// 让摄像机跟随某个节点（角色），并按场景范围夹住
    void setCameraFollowTarget(class KgtNode *target) const;
private:
    std::shared_ptr<KgtStage> stage;
    class StageCameraNode *cameraNode = nullptr;
    std::vector<ax::Node*> scriptNodes;
};

#endif //GAMESTAGE_HPP
