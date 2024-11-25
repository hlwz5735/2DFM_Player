//
// Created by limen on 2024/11/25.
//

#include "SeamlessScrollComponent.hpp"

#include "GameConfig.hpp"

USING_NS_AX;

void SeamlessScrollComponent::onAdd() {
    Component::onAdd();
    seedSprite = dynamic_cast<Sprite *>(_owner->getChildByName("SpriteComponent"));
    assert(seedSprite != nullptr);
}

void SeamlessScrollComponent::updateSprite() {
    // 移除旧内容
    if (pNode) {
        _owner->removeChild(pNode);
    }
    // FIXME: 纹理有可能为空
    auto tex = seedSprite->getSpriteFrame()->getTexture();
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
        if (cy <= 0) {
            do {
                cy += seedHeight;
            } while (cy <= 0);
        }

        if (seedHeight >= stageSize.height) {
            vertSubs = 2;
        } else {
            vertSubs = stageSize.height / seedHeight + 2;
        }
    }

    pNode = utils::createInstance<Node>();
    pNode->setPosition(cx, cy);

    for (int i = 0; i < vertSubs; ++i) {
        auto rowNode = utils::createInstance<Node>();
        rowNode->setName(std::format("seamless_row_{}", i));

        for (int j = 0; j < horiSubs; ++j) {
            auto tileNode = utils::createInstance<Sprite>();
            tileNode->setName(std::format("seamless_tile_{}", i));
            tileNode->setAnchorPoint(Vec2{0, 1});
            tileNode->setSpriteFrame(seedSprite->getSpriteFrame());
            tileNode->setPosition(j * seedWidth, i * seedHeight);
            tileNode->setOpacity(seedSprite->getOpacity());
            tileNode->setBlendFunc(seedSprite->getBlendFunc());
            rowNode->addChild(tileNode);
        }
        pNode->addChild(rowNode);
    }
    _owner->addChild(pNode);
}

void SeamlessScrollComponent::update(float delta) {
    Component::update(delta);
    if (seedWidth == 0 || seedHeight == 0) {
        return;
    }
    // 越界替换
    if (this->horiSeamless || this->vertSeamless) {
        auto nodePos = _owner->getPosition();
        int dx = nodePos.x;
        int dy = nodePos.y;
        if (this->horiSeamless) {
            if (dx >= 0) {
                dx = dx % seedWidth - seedWidth;
            } else if (dx < -seedWidth) {
                dx = dx % seedWidth;
            }
        }
        if (this->vertSeamless) {
            if (dy >= 0) {
                dy = dy % seedHeight - seedHeight;
            } else if (dy < -seedHeight) {
                dy = dy % seedHeight;
            }
        }
        _owner->setPosition(dx, dy);
    }
}
