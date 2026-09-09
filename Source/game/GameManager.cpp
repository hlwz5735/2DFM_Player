//
// Created by limen on 2024/11/23.
//

#include "GameManager.hpp"
#include <axmol.h>

USING_NS_AX;

namespace {
    static uint64_t customShaderId = 0;
    static uint64_t seamlessShaderId = 0;
}

bool GameManager::init() {
    // 幂等：ProgramManager::registerCustomProgram 对同名 shader 重复注册（非 force）会返回 0，
    // 若每次都重新赋值会把手持的 shader id 覆盖成 0，导致此后 KgtNode 绑定到错误程序而黑屏
    if (customShaderId != 0) {
        return true;
    }

    auto programManager = ProgramManager::getInstance();
    customShaderId = programManager->registerCustomProgram(
        "custom/2dfmPositionTextureColor_vs"sv, "custom/2dfmPositionTextureColor_fs"sv, VertexLayoutType::Sprite);

    seamlessShaderId = programManager->registerCustomProgram(
        "custom/seamlessScroll_vs"sv, "custom/seamlessScroll_fs"sv, VertexLayoutType::Sprite);

    auto programState = new backend::ProgramState(ProgramManager::getInstance()->loadProgram(customShaderId));
    programState->autorelease();
    return true;
}
uint64_t GameManager::getCustomShaderProgramId() const {
    return customShaderId;
}
uint64_t GameManager::getSeamlessShaderProgramId() const {
    return seamlessShaderId;
}
