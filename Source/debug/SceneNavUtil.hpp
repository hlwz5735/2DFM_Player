#pragma once

#include <axmol.h>
#include "game/scenes/MainScene.hpp"

/**
 * 在场景底部添加一个“返回菜单”按钮，点击后 replaceScene 回 MainScene（不经过 KGT 选择）。
 */
inline void addBackToMainSceneButton(ax::Scene *scene, const char *text = "[ 返回菜单 ]") {
    const auto visibleSize = ax::Director::getInstance()->getVisibleSize();

    auto label = ax::Label::createWithTTF(ax::TTFConfig("fonts/msyh.ttc", 22), text);
    label->setPosition(visibleSize.width / 2, 28);
    scene->addChild(label, 10000);

    auto listener = ax::EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [label](ax::Touch *touch, ax::Event *) {
        const auto posInNode = label->convertToNodeSpace(touch->getLocation());
        const ax::Rect rect(0, 0, label->getContentSize().width, label->getContentSize().height);
        if (rect.containsPoint(posInNode)) {
            AXLOGI("[SceneNav] TEMP-DIAG-5: back to MainScene");
            ax::Director::getInstance()->replaceScene(ax::utils::createInstance<MainScene>());
            return true;
        }
        return false;
    };
    ax::Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, label);
}
