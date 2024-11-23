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
