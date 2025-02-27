//
// Created by limen on 2024/11/23.
//

#ifndef GAMEMANAGER_HPP
#define GAMEMANAGER_HPP

#include "engine/Singleton.hpp"

namespace ax::backend {
class ProgramState;
}
struct KgtGame;

class GameManager : public Singleton<GameManager> {
public:
    bool init();

    const KgtGame *getKgtGame() const { return kgtGame; }
    void setKgtGame(const KgtGame *kgtGame) { this->kgtGame = kgtGame; }

    uint64_t getCustomShaderProgramId() const;
private:
    const KgtGame *kgtGame = nullptr;
};

#endif //GAMEMANAGER_HPP
