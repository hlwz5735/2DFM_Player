//
// Created by limen on 2025/1/9.
//

#include "KgtPlayer.hpp"

namespace {
/// 安全查找“下一个带指定标志的脚本”：起始迭代器为 end 时直接返回 end，避免 end + 1 越界（未定义行为）
template <typename It>
It findNextFlagSafe(It it, It end, ScriptSpecialFlag flag) {
    if (it == end) {
        return end;
    }
    ++it;
    while (it != end) {
        if (it->flags == flag) {
            return it;
        }
        ++it;
    }
    return end;
}
} // namespace

void KgtPlayer::initBasicScriptInfos() {
    auto begin = scripts.begin();
    auto end = scripts.end();
    auto ii = findNextWithFlag(begin + 1, end, ScriptSpecialFlag::BACKGROUND);
    standScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    forwardScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    backwardScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    jumpUpScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    jumpForwardScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    jumpBackwardScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    fallingScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    crouchScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    crouchingScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    standUpScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    crouchMoveForwardScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    crouchMoveBackwardScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    turnAroundScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    crouchTurnAroundScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    buttonBlockScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    buttonCrouchBlockScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    buttonAirBlockScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    startActionScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    victoryScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    failScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    drawGameScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    portraitScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    stageAvatarScriptId = std::distance(begin, ii);
    ii = findNextFlagSafe(ii, end, ScriptSpecialFlag::BACKGROUND);
    shadowScriptId = std::distance(begin, ii);
}