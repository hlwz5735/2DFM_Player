//
// AfterimageComponent: “残”（AFTERIMAGE）脚本项的执行者
//

#include "AfterimageComponent.hpp"

#include "2dfm/2dfmScriptItem.hpp"
#include "GameManager.hpp"
#include "KgtColor.hpp"
#include "engine/KgtNode.hpp"

#include <algorithm>
#include <random>

USING_NS_AX;

namespace {
/// 残影相对主体精灵的 Z 偏移：-1 表示画在主体之下（经典拖影观感，主体始终清晰）
constexpr int kGhostZOffset = -1;

/// [0, 1) 均匀随机数，供 RANDOM 残影使用
float randomUnit() {
    static std::mt19937 gen(std::random_device{}());
    static std::uniform_real_distribution<float> dist(0.f, 1.f);
    return dist(gen);
}
} // namespace

bool AfterimageComponent::init() {
    if (!KgtComponent::init()) {
        return false;
    }
    this->setName("AfterimageComponent");
    return true;
}

void AfterimageComponent::onExit() {
    clear();
    KgtComponent::onExit();
}

void AfterimageComponent::applyCommand(const _2dfm::AfterimageCmd *cmd) {
    if (cmd == nullptr) {
        return;
    }

    // 实测数据中 maxCount == 0 就是“停止生成残影”的写法：
    // 只停止生成，已经产生的残影继续走完自己的淡出，避免残影突然整片消失
    if (cmd->afterimageMaxCount == 0) {
        enabled = false;
        spawnTimer = 0.f;
        return;
    }

    maxCount = cmd->afterimageMaxCount;
    // 间隔单位为 1/100 秒，至少 1 个时间单位，避免除零与每帧连发
    gapSeconds = static_cast<float>(std::max<int>(cmd->afterimageGap, 1)) / 100.f;
    // 存活时间 = 残影个数 × 生成间隔
    lifeSeconds = gapSeconds * static_cast<float>(maxCount);
    blendType = cmd->getColorBlendType();
    colorType = cmd->getAfterimageColorType();
    colorR = convertKgtRgbColorValue(static_cast<byte>(cmd->red));
    colorG = convertKgtRgbColorValue(static_cast<byte>(cmd->green));
    colorB = convertKgtRgbColorValue(static_cast<byte>(cmd->blue));
    alphaOpacity = convertKgtOpacityColorValue(static_cast<byte>(cmd->alpha));
    enabled = true;
    // 下一个更新周期立刻产出第一个残影
    spawnTimer = gapSeconds;
}

void AfterimageComponent::clear() {
    for (auto &ghost : ghosts) {
        if (ghost.sprite != nullptr) {
            ghost.sprite->removeFromParent();
        }
    }
    ghosts.clear();
    enabled = false;
    spawnTimer = 0.f;
}

void AfterimageComponent::update(float delta) {
    KgtComponent::update(delta);
    ++frameCounter;

    const bool blink = colorType == _2dfm::AfterimageCmd::AfterimageColorType::LINEAR_BLINK;
    const bool blinkVisible = !blink || (frameCounter % 2) == 0;

    // ── 1. 老化、重新锚定与回收 ──
    auto *container = getOwner() != nullptr ? getOwner()->getSpriteContainer() : nullptr;
    for (auto it = ghosts.begin(); it != ghosts.end();) {
        it->age += delta;
        if (it->age >= lifeSeconds) {
            if (it->sprite != nullptr) {
                it->sprite->removeFromParent();
            }
            it = ghosts.erase(it);
            continue;
        }
        // 残影与主体精灵同容器，容器会随角色移动；用容器世界坐标的位移增量反向补偿，
        // 让残影视觉上留在生成时的位置
        if (container != nullptr) {
            const auto containerNow = container->convertToWorldSpace(Vec2::ZERO);
            it->sprite->setPosition(it->localPos - (containerNow - it->containerWorldPos));
        }
        const float progress = lifeSeconds > 0.f ? std::min(it->age / lifeSeconds, 1.f) : 1.f;
        refreshGhost(it->sprite, progress, blinkVisible);
        ++it;
    }

    if (!enabled) {
        return;
    }

    // ── 2. 按间隔生成 ──
    spawnTimer += delta;
    if (spawnTimer >= gapSeconds) {
        // 掉帧时不做补偿式连发，直接对齐到当前时刻
        spawnTimer = std::fmod(spawnTimer, gapSeconds);
        spawnGhost();
    }

    // ── 3. 超过上限则丢弃最早的残影 ──
    while (!ghosts.empty() && static_cast<int>(ghosts.size()) > maxCount) {
        if (ghosts.front().sprite != nullptr) {
            ghosts.front().sprite->removeFromParent();
        }
        ghosts.erase(ghosts.begin());
    }
}

ax::Sprite *AfterimageComponent::createGhostSprite() const {
    const auto *owner = getOwner();
    if (owner == nullptr) {
        return nullptr;
    }
    auto *sprite = owner->getSprite();
    if (sprite == nullptr) {
        return nullptr;
    }

    auto *ghost = utils::createInstance<Sprite>();
    if (ghost == nullptr) {
        return nullptr;
    }

    // setTexture 会把 rect 重置成整张纹理，所以先设置纹理再设置 rect
    ghost->setTexture(sprite->getTexture());
    ghost->setTextureRect(sprite->getTextureRect(), sprite->isTextureRectRotated(), sprite->getContentSize());
    ghost->setAnchorPoint(sprite->getAnchorPoint());
    ghost->setFlippedX(sprite->isFlippedX());
    ghost->setFlippedY(sprite->isFlippedY());
    ghost->setScaleX(sprite->getScaleX());
    ghost->setScaleY(sprite->getScaleY());
    ghost->setRotationSkewX(sprite->getRotationSkewX());
    ghost->setRotationSkewY(sprite->getRotationSkewY());
    // 颜色叠加依赖自定义着色器，与角色精灵保持一致
    ghost->setProgramStateByProgramId(GameManager::getInstance().getCustomShaderProgramId());
    ghost->setBlendFunc(resolveKgtColorBlend(blendType, alphaOpacity).blendFunc);
    return ghost;
}

void AfterimageComponent::spawnGhost() {
    auto *owner = getOwner();
    if (owner == nullptr) {
        return;
    }
    auto *sprite = owner->getSprite();
    // 主体精灵所在的节点：残影与主体精灵同节点，靠 Z 序保证画在主体之下
    auto *container = owner->getSpriteContainer();
    if (sprite == nullptr || container == nullptr || sprite->getTexture() == nullptr) {
        return;
    }

    auto *ghost = createGhostSprite();
    if (ghost == nullptr) {
        return;
    }

    // 残影与主体精灵同容器：起手位置直接取主体精灵的局部坐标（不经过世界坐标换算，
    // 避免坐标系/变换缓存带来的偏差），之后再用容器世界坐标的位移增量保持"留在原地"
    const auto localPos = sprite->getPosition();
    const auto containerWorldPos = container->convertToWorldSpace(Vec2::ZERO);
    ghost->setPosition(localPos);
    // 主体精灵之下：axmol 同节点内按 Z 序绘制，Z 更小者先画（被后画的主体覆盖）
    container->addChild(ghost, sprite->getLocalZOrder() + kGhostZOffset);

    ghosts.push_back(Ghost{ghost, localPos, containerWorldPos, 0.f});
    refreshGhost(ghost, 0.f, true);
}

void AfterimageComponent::refreshGhost(ax::Sprite *sprite, float progress, bool blinkVisible) {
    if (sprite == nullptr) {
        return;
    }

    const auto blend = resolveKgtColorBlend(blendType, alphaOpacity);
    float r = colorR;
    float g = colorG;
    float b = colorB;
    float alpha = static_cast<float>(blend.baseOpacity) / 255.f;

    switch (colorType) {
    case _2dfm::AfterimageCmd::AfterimageColorType::NONE:
        // 不施加颜色变化（混合方式仍然生效）
        r = g = b = 0.f;
        break;
    case _2dfm::AfterimageCmd::AfterimageColorType::FIXED:
        // 整段生命周期保持指定颜色
        break;
    case _2dfm::AfterimageCmd::AfterimageColorType::LINEAR_SMOOTH:
    case _2dfm::AfterimageCmd::AfterimageColorType::LINEAR_BLINK: {
        const float fade = std::clamp(1.f - progress, 0.f, 1.f);
        r *= fade;
        g *= fade;
        b *= fade;
        alpha *= fade;
        break;
    }
    case _2dfm::AfterimageCmd::AfterimageColorType::RANDOM:
        // 按各分量的方向随机；未指定透明度时不随机透明度
        r *= randomUnit();
        g *= randomUnit();
        b *= randomUnit();
        if (blendType == _2dfm::ColorBlendType::ALPHA_BLEND) {
            alpha *= randomUnit();
        }
        break;
    }

    float colorArr[3] = {r, g, b};
    if (auto *programState = sprite->getProgramState()) {
        programState->setUniform(programState->getUniformLocation("_2dfmColor"), &colorArr[0], sizeof(colorArr));
    }
    sprite->setOpacity(static_cast<uint8_t>(std::clamp(alpha, 0.f, 1.f) * 255.f));
    sprite->setVisible(blinkVisible);
}
