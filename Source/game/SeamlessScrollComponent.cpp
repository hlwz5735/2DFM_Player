//
// Created by limen on 2024/11/25.
//

#include "SeamlessScrollComponent.hpp"

#include "GameConfig.hpp"
#include "GameManager.hpp"
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
    // 清理铺贴精灵
    if (tileSprite) {
        getOwner()->spritePNode->removeChild(tileSprite);
        tileSprite = nullptr;
    }
    if (seedSprite) {
        seedSprite->release();
    }
    KgtComponent::onExit();
}

void SeamlessScrollComponent::updateSprite() {
    // 移除旧精灵
    if (tileSprite) {
        getOwner()->spritePNode->removeChild(tileSprite);
        tileSprite = nullptr;
    }
    // 获取纹理长宽信息（目前2DFM使用的精灵帧都是全幅纹理）
    const auto tex = seedSprite->getSpriteFrame()->getTexture();
    if (!tex) {
        return;
    }
    // 设置种子宽高
    const auto& r = tex->getContentSizeInPixels();
    seedWidth = r.width;
    seedHeight = r.height;

    // 着色器内 fract() 自动 wrap，无需设置纹理 REPEAT

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
            vertSubs = stageSize.height / seedHeight + 3;
        }
    }

    // 单个精灵 + 自定义着色器 u_tileScale 控制平铺，替代 N×M 子精灵网格
    tileSprite = utils::createInstance<Sprite>();
    tileSprite->setAnchorPoint(Vec2{0, 1});
    tileSprite->setSpriteFrame(seedSprite->getSpriteFrame());
    tileSprite->setTextureRect({0, 0, static_cast<float>(seedWidth), static_cast<float>(seedHeight)});
    tileSprite->setContentSize(
        {static_cast<float>(seedWidth * horiSubs), static_cast<float>(seedHeight * vertSubs)});
    tileSprite->setProgramStateByProgramId(GameManager::getInstance().getSeamlessShaderProgramId());

    // 设置着色器 uniform：平铺倍数
    auto ps = tileSprite->getProgramState();
    auto loc = ps->getUniformLocation("u_tileScale");
    float tileScale[2] = {static_cast<float>(horiSubs), static_cast<float>(vertSubs)};
    ps->setUniform(loc, tileScale, sizeof(tileScale));

    tileSprite->setPosition(cx, cy);
    tileSprite->setOpacity(seedSprite->getOpacity());
    tileSprite->setBlendFunc(seedSprite->getBlendFunc());
    tileSprite->setFlippedX(seedSprite->isFlippedX());
    tileSprite->setFlippedY(seedSprite->isFlippedY());

    getOwner()->spritePNode->addChild(tileSprite);
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
        float dy = nodePos.y;
        if (this->horiSeamless) {
            if (dx >= 0) {
                dx -= ((static_cast<int>(dx) / seedWidth) + 1) * seedWidth;
            } else if (dx < -seedWidth) {
                dx += (static_cast<int>(abs(dx)) / seedWidth) * seedWidth;
            }
        }
        if (this->vertSeamless) {
            if (dy >= 0) {
                dy = fmodf(dy, seedHeight) - seedHeight;
            } else if (dy < -seedHeight) {
                dy = fmodf(dy, seedHeight);
            }
            // anchor(0,1) 下内容向下延伸，需比横向多提升一个分段
            dy += seedHeight;
        }
        getOwner()->spritePNode->setPosition(Vec2(dx, dy));
    }
}
