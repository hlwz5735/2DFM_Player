#include "StageScriptInterceptor.hpp"
#include "2dfm/2dfmScriptItem.hpp"
#include "2dfm/KgtStage.hpp"
#include "MoveComponent.hpp"
#include "engine/AudioSystem.hpp"
#include "engine/SoundClip.hpp"
#include <2d/Node.h>

USING_NS_AX;

namespace {
uint8_t convertKgtRgbColorValue(byte rawByte) {
    const int8_t val = *(reinterpret_cast<char *>(&rawByte));
    return static_cast<uint8_t>(val + 32);
}

uint8_t convertKgtOpacityColorValue(byte rawByte) {
    const auto val = 32.f - static_cast<int>(rawByte);
    return static_cast<uint8_t>((val / 32.f) * 255);
}
}

bool StageScriptInterceptor::init() {
    if (!Component::init()) {
        return false;
    }
    return true;
}

void StageScriptInterceptor::onAdd() {
    Component::onAdd();
    spriteComponent = dynamic_cast<Sprite *>(_owner->getChildByName("SpriteComponent"));
    moveComponent = dynamic_cast<MoveComponent *>(_owner->getComponent("MoveComponent"));

    if (!spriteComponent || !moveComponent) {
        throw std::runtime_error("Can't get required components!");
    }
}

void StageScriptInterceptor::update(float deltaTime) {
    Component::update(deltaTime);
    if (!spriteComponent || !stageData) {
        return;
    }
    if (timeWaiting == std::numeric_limits<float>::infinity()) {
        return;
    }
    if (runningScriptItemIdx == endIdx) {
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
            runningScriptItemIdx = endIdx;
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

void StageScriptInterceptor::setRunningScript(int scriptIdx) {
    auto &script = stageData->scripts.at(scriptIdx);
    startIdx = script.startIdx;
    endIdx = script.endIdx;
    runningScriptItemIdx = startIdx;
}

void StageScriptInterceptor::interceptPlaySoundCmd(const _2dfm::PlaySoundCmd *cmd) {
    auto soundClip = stageData->sounds.at(cmd->soundIdx);
    if (soundClip) {
        AudioSystem::getInstance()->playClip(soundClip, soundClip->isLoop(), 1.f);
    }
}

bool StageScriptInterceptor::hasNoShowPicItem() const {
    for (int i = startIdx; i < endIdx; ++i) {
        auto item = stageData->scriptItems[i];
        if (static_cast<_2dfm::CommonScriptItemTypes>(item->type) == _2dfm::CommonScriptItemTypes::PIC) {
            return false;
        }
    }
    return true;
}

_2dfm::ShowPic *StageScriptInterceptor::interceptScriptUntilShowPic() {
    for (int i = runningScriptItemIdx + 1; i < endIdx; ++i) {
        auto item = stageData->scriptItems[i];
        auto type = static_cast<_2dfm::CommonScriptItemTypes>(item->type);

        if (type == _2dfm::CommonScriptItemTypes::PIC) { // 图
            runningScriptItemIdx = i;
            return reinterpret_cast<_2dfm::ShowPic *>(item);
        }
        if (type == _2dfm::CommonScriptItemTypes::END) { // 完
            spriteComponent->setTexture(nullptr);
            spriteComponent->setVisible(false);
            return nullptr;
        }

        if (type == _2dfm::CommonScriptItemTypes::SOUND) { // 声
            interceptPlaySoundCmd(reinterpret_cast<_2dfm::PlaySoundCmd *>(item));
        } else if (type == _2dfm::CommonScriptItemTypes::COLOR) { // 色
            interceptColorSetCmd(reinterpret_cast<_2dfm::ColorSet *>(item));
        } else if (type == _2dfm::CommonScriptItemTypes::MOVE) { // 移
            interceptMoveCmd(reinterpret_cast<_2dfm::MoveCmd *>(item));
        }
    }
    // 如果走到了最后，发现从头到尾都没有图片指令，则退出播放
    if (hasNoShowPicItem()) {
        spriteComponent->setVisible(false);
        return nullptr;
    } else {
        // 只要没遇到“完”，就会从头开始
        runningScriptItemIdx = startIdx;
        return interceptScriptUntilShowPic();
    }
}

void StageScriptInterceptor::interceptShowPicCmd(const _2dfm::ShowPic *cmd) {
    auto tex = stageData->pictures.at(cmd->getPicIdx());
    if (tex) {
        const auto blendFunc = spriteComponent->getBlendFunc();
        const auto visible = spriteComponent->isVisible();
        spriteComponent->setTexture(tex);
        const auto &r = tex->getContentSizeInPixels();
        spriteComponent->setTextureRect({0, 0, r.width, r.height});
        spriteComponent->setPosition(cmd->getOffset().x, -cmd->getOffset().y);
        spriteComponent->setVisible(visible);
        spriteComponent->setBlendFunc(blendFunc);
    } else {
        spriteComponent->setTexture(nullptr);
        spriteComponent->setVisible(false);
    }
}

void StageScriptInterceptor::interceptColorSetCmd(const _2dfm::ColorSet *cmd) {
    auto setType = static_cast<_2dfm::ColorSetType>(cmd->colorSetType);
    switch (setType) {
    case _2dfm::ColorSetType::ALPHA_BLEND: {
        auto opacity = convertKgtOpacityColorValue(cmd->alpha);
        if (opacity == 0) {
            spriteComponent->setOpacity(0);
            spriteComponent->setVisible(false);
        } else {
            spriteComponent->setBlendFunc(BlendFunc::ALPHA_NON_PREMULTIPLIED);
            spriteComponent->setOpacity(opacity);
            spriteComponent->setVisible(true);
        }
    } break;
    case _2dfm::ColorSetType::TRANSPARENCY:
        spriteComponent->setVisible(true);
        spriteComponent->setBlendFunc(BlendFunc::ALPHA_NON_PREMULTIPLIED);
        spriteComponent->setOpacity(128);
        break;
    case _2dfm::ColorSetType::ADD_BLEND:
        spriteComponent->setVisible(true);
        spriteComponent->setBlendFunc(BlendFunc::ADDITIVE);
        spriteComponent->setOpacity(255);
        break;
    case _2dfm::ColorSetType::MINUS_BLEND:
        spriteComponent->setVisible(true);
        spriteComponent->setBlendFunc({backend::BlendFactor::ONE, backend::BlendFactor::ONE_MINUS_SRC_COLOR});
        spriteComponent->setOpacity(255);
        break;
    case _2dfm::ColorSetType::NORMAL:
    default:
        spriteComponent->setVisible(true);
        spriteComponent->setBlendFunc(BlendFunc::ALPHA_NON_PREMULTIPLIED);
        spriteComponent->setOpacity(255);
        break;
    }
    // TODO: 颜色叠加暂时关闭
    /*const ax::Color3B blendColor(convertKgtRgbColorValue(colorSet->red),
        convertKgtRgbColorValue(colorSet->green),
        convertKgtRgbColorValue(colorSet->blue));
    spriteComponent->setColor(blendColor);*/
}
void StageScriptInterceptor::interceptMoveCmd(const _2dfm::MoveCmd *cmd){
    auto vel       = ax::Vec2(cmd->moveX, cmd->moveY) * 0.01f;
    auto accel     = ax::Vec2(cmd->accelX, cmd->accelY) * 0.01f;
    auto &oriVel = moveComponent->getVelocity();
    auto &oriAccel = moveComponent->getAcceleration();
    if (cmd->isAdd()) {
        // 相加模式
        vel.x += (cmd->isIgnoreMoveX() ? 0 : oriVel.x);
        vel.y += (cmd->isIgnoreMoveY() ? 0 : oriVel.y);
        accel.x += (cmd->isIgnoreAccelX() ? 0 : oriAccel.x);
        accel.y += (cmd->isIgnoreAccelY() ? 0 : oriAccel.y);
    } else {
        // 代入模式
        vel.x = cmd->isIgnoreMoveX() ? oriVel.x : vel.x;
        vel.y = cmd->isIgnoreMoveY() ? oriVel.y : vel.y;
        accel.x = cmd->isIgnoreAccelX() ? oriAccel.x : accel.x;
        accel.y = cmd->isIgnoreAccelY() ? oriAccel.y : accel.y;
    }
    moveComponent->setAcceleration(accel);
    moveComponent->setVelocity(vel);
}
