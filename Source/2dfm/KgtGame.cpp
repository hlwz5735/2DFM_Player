#include "KgtGame.hpp"

KgtGame::~KgtGame() {}

std::string KgtGame::getOpeningDemoName() const {
    int idx = static_cast<int>(demoConfig.openingDemoId) - 1;
    return demoNames[idx];
}
std::string KgtGame::getTitleDemoName() const {
    int idx = static_cast<int>(demoConfig.titleDemoId) - 1;
    return demoNames[idx];
}
std::string KgtGame::getCharSelectionDemoName() const {
    // TODO: 这里需要考虑区分故事模式、1v1模式和组队模式使用的DEMO可能不同
    int idx = static_cast<int>(demoConfig.storyModeCharSelectDemoId) - 1;
    return demoNames[idx];
}
