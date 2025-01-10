#include "KgtGame.hpp"

#include "2dfmScriptItem.hpp"

KgtGame::~KgtGame() {}

ax::Vec2 KgtGame::getTitleStoryModePos() const {
    const auto &script = scripts[this->storyModePositionScriptId];
    const auto scriptItem = scriptItems[script.startIdx];
    return reinterpret_cast<_2dfm::KgtPos *>(scriptItem)->getPosition();
}
ax::Vec2 KgtGame::getTitlePvpModePos() const {
    const auto &script = scripts[this->pvpModePositionScriptId];
    const auto scriptItem = scriptItems[script.startIdx];
    return reinterpret_cast<_2dfm::KgtPos *>(scriptItem)->getPosition();
}
ax::Vec2 KgtGame::getTitleTeamModePos() const {
    const auto &script = scripts[this->teamModePositionScriptId];
    const auto scriptItem = scriptItems[script.startIdx];
    return reinterpret_cast<_2dfm::KgtPos *>(scriptItem)->getPosition();
}

void KgtGame::initBasicScriptInfos() {
    auto begin = scripts.begin();
    auto end = scripts.end();
    auto ii = findNextWithFlag(begin, end, ScriptSpecialFlag::HIT_SYMBOL);
    this->comboHitScriptId = std::distance(begin, ii);
    for (int i = 0; i < 10; ++i) {
        ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::COMBO_SYMBOL);
        this->comboNumberScriptIds[i] = std::distance(begin, ii);
    }

    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::ROUND);
    this->roundStartScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::ROUND);
    this->roundEndScriptId = std::distance(begin, ii);
    for (int i = 1; i < 10; ++i) {
        ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::ROUND);
        this->roundNumberScriptIds[i] = std::distance(begin, ii);
    }
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::ROUND);
    this->finalRoundScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::ROUND);
    this->fightScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::ROUND);
    this->koScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::ROUND);
    this->perfectGameScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::ROUND);
    this->youWinScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::ROUND);
    this->youLoseScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::ROUND);
    this->player1WinScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::ROUND);
    this->player2WinScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::ROUND);
    this->drawGameScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::ROUND);
    this->doubleKillScriptId = std::distance(begin, ii);

    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::TIME_NUMBER);
    this->timeNumberInfinityScriptId = std::distance(begin, ii);
    for (int i = 0; i < 10; ++i) {
        ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::TIME_NUMBER);
        this->timeNumberScriptIds[i] = std::distance(begin, ii);
    }
    for (int i = 0; i < 10; ++i) {
        ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::SKILL_POINT_NUMBER);
        this->skillPointNumberScriptIds[i] = std::distance(begin, ii);
    }

    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::VICTORY_FLAG);
    this->victorySymbolFillScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::VICTORY_FLAG);
    this->victorySymbolOutlineScriptId = std::distance(begin, ii);

    for (int i = 0; i < 10; ++i) {
        ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::STAGE_MAIN_UI);
        this->stageGuiScripts[i] = std::distance(begin, ii);
    }
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::STAGE_MAIN_UI);
    this->player1HpBarScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::STAGE_MAIN_UI);
    this->player2HpBarScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::STAGE_MAIN_UI);
    this->player1SpBarScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::STAGE_MAIN_UI);
    this->player2SpBarScriptId = std::distance(begin, ii);

    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::TIMER_POS);
    this->timerPositionScriptId = std::distance(begin, ii);

    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::PLAYER_1_AVATAR_POS);
    this->player1AvatarPositionScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::PLAYER_2_AVATAR_POS);
    this->player2AvatarPositionScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::PLAYER_1_SKILL_POINT_POS);
    this->player1SkillPointPositionScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::PLAYER_2_SKILL_POINT_POS);
    this->player2SkillPointPositionScriptId = std::distance(begin, ii);

    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::PLAYER_1_VICTORY_POS);
    this->player1VictoryFlagPositionScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::PLAYER_2_VICTORY_POS);
    this->player2VictoryFlagPositionScriptId = std::distance(begin, ii);

    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    this->titleCursorScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::SYSTEM);
    this->storyModePositionScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::SYSTEM);
    this->pvpModePositionScriptId = std::distance(begin, ii);

    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    this->continueCursorScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::SYSTEM);
    this->continuePositionScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::SYSTEM);
    this->giveUpPositionScriptId = std::distance(begin, ii);

    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    this->player1CharSelCursorScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    this->player2CharSelCursorScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    this->player1CharSelConfirmCursorScriptId = std::distance(begin, ii);
    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    this->player2CharSelConfirmCursorScriptId = std::distance(begin, ii);

    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::SYSTEM);
    this->teamModePositionScriptId = std::distance(begin, ii);

    ii = findNextWithFlag(ii + 1, end, ScriptSpecialFlag::BACKGROUND);
    this->pauseScriptId = std::distance(begin, ii);
}

bool KgtGame::isPlayerInStoryMode(int playerNo) const {
    if (playerNo >= _2dfm::maxPlayerNum) {
        return false;
    }
    return playerSelectableInfos[playerNo] & 0x01;
}
bool KgtGame::isPlayerInPvpMode(int playerNo) const{
    if (playerNo >= _2dfm::maxPlayerNum) {
        return false;
    }
    return playerSelectableInfos[playerNo] & 0x02;
}


