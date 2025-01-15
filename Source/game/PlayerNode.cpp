//
// Created by limen on 2025/1/10.
//

#include "PlayerNode.hpp"

#include "MoveComponent.hpp"
#include "ScriptInterceptorComponent.hpp"

USING_NS_AX;

bool PlayerNode::initWithVisibleHeight(int visibleHeight) {
    if (!Node::init()) {
        return false;
    }
    this->visibleHeight = visibleHeight;

    moveComp = utils::createInstance<MoveComponent>();
    moveComp->setName("MoveComponent");
    this->addComponent(moveComp);

    spritePNode = utils::createInstance<Node>();
    this->addChild(spritePNode);

    spriteComp = utils::createInstance<Sprite>();
    spriteComp->setName("SpriteComponent");
    spriteComp->setPosition(Vec2::ZERO);
    spriteComp->setAnchorPoint(Vec2(0.5f, 0));
    spritePNode->addChild(spriteComp);

    for (int i = 0; i < 10; ++i) {
        managedObjects.emplace_back(nullptr);
    }

    return true;
}

void PlayerNode::addObjectNode(PlayerNode *node, int manageNo) {
    // 如果物体是受管理号管理的
    if (manageNo != -1) {
        AXASSERT(manageNo >= 0 && manageNo < 10, "管理号超限！");
        auto &objNode = managedObjects[manageNo];
        if (objNode != nullptr) {
            // TODO: 已存在逻辑
            objNode->destroy();
            objNode = nullptr;
        }
        objNode = node;
    } else {
        auto b = managedObjects.begin() + 9;
        const auto e = managedObjects.end();
        while (++b != e) {
            if (*b == nullptr) {
                break;
            }
        }
        if (b == e) {
            managedObjects.emplace_back(node);
        } else {
            *b = node;
        }
    }
}

void PlayerNode::removeObjectNode(PlayerNode *node) {
    auto i = std::find(managedObjects.begin(), managedObjects.end(), node);
    AXASSERT(i != managedObjects.end(), "要移除的物件Node不存在！");
    *i = nullptr;
}

void PlayerNode::destroyManagedObjects() {
    for (auto &pNode : managedObjects) {
        if (pNode != nullptr) {
            pNode->destroy();
            pNode = nullptr;
        }
    }
}
