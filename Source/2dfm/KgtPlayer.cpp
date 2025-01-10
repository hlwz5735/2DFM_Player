//
// Created by limen on 2025/1/9.
//

#include "KgtPlayer.hpp"

void KgtPlayer::initBasicScriptInfos() {
    auto begin = scripts.begin();
    auto end = scripts.end();
    auto ii = findNextWithFlag(begin + 1, end, ScriptSpecialFlag::BACKGROUND);
    standScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    forwardScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    backwardScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    jumpUpScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    jumpForwardScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    jumpBackwardScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    fallingScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    crouchScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    crouchingScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    standUpScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    crouchMoveForwardScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    crouchMoveBackwardScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    turnAroundScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    crouchTurnAroundScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    buttonBlockScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    buttonCrouchBlockScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    buttonAirBlockScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    startActionScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    victoryScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    failScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    drawGameScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    portraitScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    stageAvatarScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    shadowScriptId = std::distance(begin, ii);
}