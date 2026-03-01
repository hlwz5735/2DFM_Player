//
// Created by limen on 2024/11/23.
//

#ifndef GAMEMANAGER_HPP
#define GAMEMANAGER_HPP

#include "engine/Singleton.hpp"

struct KgtStage;
namespace ax::backend {
class ProgramState;
}
struct KgtGame;

class GameManager : public Singleton<GameManager> {
public:
    bool init();

    const KgtGame *getKgtGame() const { return kgtGame; }
    const KgtStage *getKgtStage() const { return kgtStage; }
    void setKgtGame(const KgtGame *kgtGame) { this->kgtGame = kgtGame; }
    void setKgtStage(const KgtStage *kgtStage) { this->kgtStage = kgtStage; }

    uint64_t getCustomShaderProgramId() const;
private:
    const KgtGame *kgtGame = nullptr;
    const KgtStage *kgtStage = nullptr;
};

#endif //GAMEMANAGER_HPP
