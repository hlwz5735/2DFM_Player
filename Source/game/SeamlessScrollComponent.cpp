//
// Created by limen on 2024/11/25.
//

#include "SeamlessScrollComponent.hpp"

#include "GameConfig.hpp"
#include "engine/KgtNode.hpp"
#include <axmol.h>

USING_NS_AX;

bool SeamlessScrollComponent::init() {
    if (!KgtComponent::init()) {
        return false;
    }
    this->setName("SeamlessScrollComponent");
    return true;
}
void SeamlessScrollComponent::onAdd() {
    Component::onAdd();
    seedSprite = getOwner()->getSprite();
    seedSprite->retain();
    getOwner()->spritePNode->removeChild(seedSprite);
    AXASSERT(seedSprite != nullptr, "seedSprite can't be null.");
}
void SeamlessScrollComponent::onExit() {
    KgtComponent::onExit();
    if (seedSprite) {
        seedSprite->release();
    }
}

void SeamlessScrollComponent::updateSprite() {
    // 移除旧内容
    if (rootNode) {
        getOwner()->spritePNode->removeChild(rootNode);
    }
    // 获取纹理长宽信息（目前2DFM使用的精灵帧都是全幅纹理）
    const auto tex = seedSprite->getSpriteFrame()->getTexture();
    if (!tex) {
        return;
    }
    // 设置种子宽高
    const auto &r = tex->getContentSizeInPixels();
    seedWidth = r.width;
    seedHeight = r.height;

    auto cx = seedSprite->getPosition().x;
    auto cy = seedSprite->getPosition().y;
    int horiSubs = 1;
    int vertSubs = 1;

    auto stageSize = GameConfig::getInstance().getStageSize();
    if (this->horiSeamless) {
        if (cx > 0) {
            do {
                cx -= seedWidth;
            } while (cx > 0);
        }

        if (seedWidth >= stageSize.width) {
            horiSubs = 2;
        } else {
            horiSubs = stageSize.width / seedWidth + 2;
        }
    }

    if (this->vertSeamless) {
        if (cy > 0) {
            do {
                cy -= seedHeight;
            } while (cy > 0);
        }

        if (seedHeight >= stageSize.height) {
            vertSubs = 2;
        } else {
            vertSubs = stageSize.height / seedHeight + 2;
        }
    }

    rootNode = utils::createInstance<Node>();
    rootNode->setPosition(cx, cy);

    for (int i = 0; i < vertSubs; ++i) {
        auto rowNode = utils::createInstance<Node>();
        rowNode->setName(std::format("seamless_row_{}", i));

        for (int j = 0; j <= horiSubs; ++j) {
            auto tileNode = utils::createInstance<Sprite>();
            tileNode->setName(std::format("seamless_tile_{}", i));
            tileNode->setAnchorPoint(Vec2{0, 1});
            tileNode->setSpriteFrame(seedSprite->getSpriteFrame());
            tileNode->setPosition(j * seedWidth, i * seedHeight);
            tileNode->setOpacity(seedSprite->getOpacity());
            tileNode->setBlendFunc(seedSprite->getBlendFunc());
            tileNode->setFlippedX(seedSprite->isFlippedX());
            tileNode->setFlippedY(seedSprite->isFlippedY());

            rowNode->addChild(tileNode);
        }
        rootNode->addChild(rowNode);
    }
    getOwner()->spritePNode->addChild(rootNode);
    if (seedSprite->isVisible()) {
        seedSprite->setVisible(false);
    }
}

void SeamlessScrollComponent::lateUpdate(float delta) {
    KgtComponent::lateUpdate(delta);
    if (seedWidth == 0 || seedHeight == 0) {
        return;
    }

    // 越界替换
    if (this->horiSeamless || this->vertSeamless) {
        auto nodePos = getOwner()->spritePNode->getPosition();
        float dx = nodePos.x;
        int dy = nodePos.y;
        if (this->horiSeamless) {
            if (dx >= 0) {
                dx -= ((static_cast<int>(dx) / seedWidth) + 1) * seedWidth;
            } else if (dx < -seedWidth) {
                dx += (static_cast<int>(abs(dx)) / seedWidth) * seedWidth;
            }
        }
        if (this->vertSeamless) {
            if (dy >= 0) {
                dy = dy % seedHeight - seedHeight;
            } else if (dy < -seedHeight) {
                dy = dy % seedHeight;
            }
        }
        getOwner()->spritePNode->setPosition(Vec2(dx, nodePos.y));
    }
}
