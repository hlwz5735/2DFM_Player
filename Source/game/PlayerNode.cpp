//
// Created by limen on 2025/1/10.
//

#include "PlayerNode.hpp"
#include "AfterimageComponent.hpp"
#include "GameManager.hpp"
#include "MoveComponent.hpp"
#include "ScriptInterceptorComponent.hpp"
#include "StageCameraNode.hpp"

USING_NS_AX;

bool PlayerNode::initWithVisibleHeight(int visibleHeight) {
    if (!Node::init()) {
        return false;
    }
    this->visibleHeight = visibleHeight;

    moveComp = utils::createInstance<MoveComponent>();
    moveComp->setName("MoveComponent");
    this->addComponent(moveComp);

    // 与 KgtNode::initWithVisibleHeight 一致：残影组件在创建阶段挂好，
    // 不能在解释器执行“残”脚本项时才 addComponent（会破坏组件遍历）
    afterimageComp = utils::createInstance<AfterimageComponent>();
    this->addComponent(afterimageComp);

    spritePNode = utils::createInstance<Node>();
    this->addChild(spritePNode);

    spriteComp = utils::createInstance<Sprite>();
    spriteComp->setName("SpriteComponent");
    spriteComp->setPosition(Vec2::ZERO);
    spriteComp->setAnchorPoint(Vec2(0.5f, 0));
    spriteComp->setProgramStateByProgramId(GameManager::getInstance().getCustomShaderProgramId());
    spritePNode->addChild(spriteComp);

    for (int i = 0; i < 10; ++i) {
        managedObjects.emplace_back(nullptr);
    }

    return true;
}

void PlayerNode::update(float delta) {
    // 延迟回收：“物”脚本结束的那一帧只做了标记，这里（下一帧、自己的 update 开头、
    // 还没有任何组件遍历压在自己身上）才真正从父节点摘掉。
    if (pendingRecycle) {
        if (objectManager != nullptr) {
            objectManager->removeObjectNode(this);
            objectManager = nullptr;
        }
        destroy();
        // destroy() 之后本对象可能已经被释放，不能再碰任何成员
        return;
    }

    // 组件先更新：MoveComponent 在这里把加速度积分进速度
    Node::update(delta);

    // 与 KgtNode::update 的区别：角色自身的位移直接积分到 logicPosition 上。
    // logicPosition 因此就是角色真正的世界坐标（浮空/触地判定都以它为准），
    // 屏幕位置再一次性由它算出，效果与原来“积到 spritePNode 上”完全一致。
    const auto &velocity = moveComp->getVelocity();
    logicPosition.x += velocity.x * delta * 100.f;
    logicPosition.y += velocity.y * delta * 100.f;

    const float viewX = viewCamera != nullptr ? viewCamera->getPositionX() : 0.f;
    const float viewY = viewCamera != nullptr ? viewCamera->getPositionY() : 0.f;
    setPosition(logicPosition.x - viewX, visibleHeight - logicPosition.y - viewY);
}

void PlayerNode::destroy() {
    // 先断开受管理物体指向自己的回指，避免它们在之后回收时访问已销毁的归属者
    for (auto *object : managedObjects) {
        if (object != nullptr && object->getObjectManager() == this) {
            object->setObjectManager(nullptr);
        }
    }
    KgtNode::destroy();
}

void PlayerNode::addObjectNode(PlayerNode *node, int manageNo) {
    // 记下归属者：物体结束回收时要先把自己从这张表里摘掉，避免留下野指针
    node->setObjectManager(this);
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
