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

    /// 添加物体Node
    void addObjectNode(PlayerNode *node, int manageNo);
    void removeObjectNode(PlayerNode *node);

    bool getIsObjectNode() const { return isObjectNode; }
    void setIsObjectNode(bool val) { isObjectNode = val; }

    void destroyManagedObjects();
private:
    /// 受管理的已生成的物体节点，前9个是固定受管理的，之后则为无条件出现的
    std::vector<PlayerNode *> managedObjects;
    bool isObjectNode = false;
};

#endif //PLAYERNODE_HPP
