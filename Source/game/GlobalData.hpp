//
// Created by limen on 2024/8/1.
//

#ifndef GLOBALDATA_HPP
#define GLOBALDATA_HPP
#include "GameConfig.hpp"

struct KgtGame;
class GlobalData {
public:
    GlobalData();
    ~GlobalData();

    static GlobalData *INSTANCE;
    static GlobalData *getInstance();

    [[nodiscard]] const GameConfig &getGameConfig() const { return gameConfig; }
    void setKgtGame(KgtGame *kgt) { kgtGame = kgt; }
    KgtGame *getKgtGame() { return kgtGame; }
private:
    GameConfig gameConfig;
    KgtGame *kgtGame = nullptr;
};

#endif //GLOBALDATA_HPP
