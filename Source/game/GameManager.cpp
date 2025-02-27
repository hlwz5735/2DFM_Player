//
// Created by limen on 2024/11/23.
//

#include "GameManager.hpp"
#include <axmol.h>

USING_NS_AX;

namespace {
    static uint64_t customShaderId = 0;
}

bool GameManager::init() {
    auto programManager = ProgramManager::getInstance();
    customShaderId = programManager->registerCustomProgram(
        "custom/2dfmPositionTextureColor_vs"sv, "custom/2dfmPositionTextureColor_fs"sv, VertexLayoutType::Sprite);

    auto programState = new backend::ProgramState(ProgramManager::getInstance()->loadProgram(customShaderId));
    programState->autorelease();
    return true;
}
uint64_t GameManager::getCustomShaderProgramId() const {
    return customShaderId;
}
