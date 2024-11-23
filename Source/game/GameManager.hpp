//
// Created by limen on 2024/11/23.
//

#ifndef GAMEMANAGER_HPP
#define GAMEMANAGER_HPP

#include "engine/Singleton.hpp"

struct KgtGame;

class GameManager : public Singleton<GameManager> {
public:
    const KgtGame *getKgtGame() const { return kgtGame; }
    void setKgtGame(const KgtGame *kgtGame) { this->kgtGame = kgtGame; }
private:
    const KgtGame *kgtGame = nullptr;
};



#endif //GAMEMANAGER_HPP
