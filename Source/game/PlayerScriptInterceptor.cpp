//
// Created by limen on 2025/1/9.
//

#include "PlayerScriptInterceptor.hpp"

#include "2dfm/2dfmScriptItem.hpp"
#include "PlayerNode.hpp"

USING_NS_AX;

void PlayerScriptInterceptor::update(float deltaTime) {
    ScriptInterceptorComponent::update(deltaTime);
    bool valid = false;
    do {
        if (!spriteComponent || !playerData) {
            break;
        }
        if (timeWaiting == std::numeric_limits<float>::infinity()) {
            break;
        }
        if (runningStack.empty()) {
            break;
        }
        if (runningScriptItemIdx == runningStack.back().scriptItemEndIdx) {
            break;
        }
        valid = true;
    } while (false);

    auto playerNode = getPlayerNode();
    if (!valid && playerNode && playerNode->getIsObjectNode()) {
        // TODO: 这里只是停止了播放
        this->stop();
        return;
    }

    if (timeWaiting > 0) {
        timeWaiting -= deltaTime;
    }
    const _2dfm::ShowPic *showPicScript = nullptr;
    while (timeWaiting <= 0) {
        showPicScript = interceptScriptUntilShowPic();
        if (!showPicScript) {
            timeWaiting = std::numeric_limits<float>::infinity();
            runningScriptItemIdx = runningStack.back().scriptItemEndIdx;
            return;
        }
        float keepTime = static_cast<float>(showPicScript->keepTime) / 100;
        if (keepTime == 0) {
            timeWaiting = std::numeric_limits<float>::infinity();
        } else {
            timeWaiting += keepTime;
        }
    }
    if (showPicScript) {
        this->interceptShowPicCmd(showPicScript);
    }
}

void PlayerScriptInterceptor::setPlayerData(KgtPlayer *playerData) {
    this->stop();
    this->playerData = playerData;
}

PlayerNode *PlayerScriptInterceptor::getPlayerNode() const {
    if (!playerNode) {
        playerNode = dynamic_cast<PlayerNode *>(getOwner());
    }
    return playerNode;
}

void PlayerScriptInterceptor::interceptObjectCmd(const _2dfm::ObjectCmd *cmd) const {
    ScriptInterceptorComponent::interceptObjectCmd(cmd);
    if (const auto playerNode = getPlayerNode()) {
        auto parentNode = playerNode->getParent();
        AXASSERT(parentNode != nullptr, "玩家Node的父节点不应为空。");
        // TODO: 似乎需要考虑场景高度问题？
        auto newPlayerNode = utils::createInstance<PlayerNode>();
        newPlayerNode->setIsObjectNode(true);
        const auto interceptor = utils::createInstance<PlayerScriptInterceptor>();
        interceptor->setPlayerData(this->playerData);
        interceptor->initRunningScript(cmd->targetScriptId, cmd->targetPos);
        newPlayerNode->addInterceptor(interceptor);
        newPlayerNode->scheduleUpdate();
        if (cmd->isUseWindowPosition()) {
            newPlayerNode->setLogicPosition(cmd->getPosition());
        } else {
            newPlayerNode->setLogicPosition(playerNode->getLogicPosition() + cmd->getPosition());
        }
        // newPlayerNode->setLocalZOrder(999);
        parentNode->addChild(newPlayerNode);
        playerNode->addObjectNode(newPlayerNode, cmd->getManageNo());
    }

}
