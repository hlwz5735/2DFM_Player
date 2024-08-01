#pragma once

#include <2d/Component.h>
#include <2d/Sprite.h>

struct KgtStage;

namespace _2dfm {
    struct PlaySoundCmd;
    struct ShowPic;
    struct ColorSet;
    struct MoveCmd;
}

class StageScriptInterceptor : public ax::Component {
public:
    bool init() override;
    void onAdd() override;
    void update(float deltaTime) override;

    void setStageData(KgtStage *data) { stageData = data; }
    void setRunningScript(int scriptIdx);
protected:
    void interceptPlaySoundCmd(const _2dfm::PlaySoundCmd *cmd);
    void interceptShowPicCmd(const _2dfm::ShowPic *cmd);
    void interceptColorSetCmd(const _2dfm::ColorSet *cmd);
    void interceptMoveCmd(const _2dfm::MoveCmd *cmd);
private:
    bool hasNoShowPicItem() const;
    _2dfm::ShowPic *interceptScriptUntilShowPic();

    ax::Sprite *spriteComponent = nullptr;
    class MoveComponent *moveComponent = nullptr;
    KgtStage *stageData = nullptr;

    int runningScriptItemIdx = 0;
    int startIdx = 0;
    int endIdx = 0;

    float timeWaiting = 0;
};
