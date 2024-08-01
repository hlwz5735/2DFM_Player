//
// Created by limen on 2024/8/1.
//

#ifndef GLOBALDATA_HPP
#define GLOBALDATA_HPP
#include "GameConfig.hpp"

struct KgtGame;
class GlobalData {
public:
    GlobalData(const GlobalData &o) = delete;
    GlobalData & operator=(const GlobalData &o) = delete;


    [[nodiscard]] const GameConfig &getGameConfig() const { return gameConfig; }
    void setKgtGame(KgtGame *kgt) { kgtGame = kgt; }
    KgtGame *getKgtGame() { return kgtGame; }

    static GlobalData *getInstance();
protected:
    GlobalData();
    ~GlobalData();
private:
    GameConfig gameConfig;
    KgtGame *kgtGame = nullptr;
};

#endif //GLOBALDATA_HPP
