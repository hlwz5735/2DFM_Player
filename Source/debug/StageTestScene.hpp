//
// Created by limen on 2024/8/1.
//

#ifndef STAGETESTSCENE_HPP
#define STAGETESTSCENE_HPP

#include <axmol.h>

class GameStage;

class StageTestScene final : public ax::Scene {
public:
    bool init() override;

    void update(float delta) override;

    void loadStage(int stageNo) const;
    void unloadStage() const;
private:
    GameStage *stage = nullptr;
    int stageNo = 0;
};

#endif //STAGETESTSCENE_HPP
