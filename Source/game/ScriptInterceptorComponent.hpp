//
// Created by limen on 2024/11/22.
//

#ifndef SCRIPTINTERCEPTORCOMPONENT_HPP
#define SCRIPTINTERCEPTORCOMPONENT_HPP

#include <axmol.h>
#include "engine/KgtComponent.hpp"

struct CommonResource;
namespace _2dfm {
    struct PlaySoundCmd;
    struct ShowPic;
    struct ColorSet;
}

struct ScriptRunningInfo {
    int runningScriptItemIdx = 0;
    int scriptItemStartIdx = 0;
    int scriptItemEndIdx = 0;
};

class ScriptInterceptorComponent : public KgtComponent {
public:
    bool init() override;
    void onAdd() override;

    void initRunningScript(int scriptIdx);

    virtual const CommonResource *getCommonResource() const = 0;

protected:
    bool hasNoShowPicItem() const;
    _2dfm::ShowPic *interceptScriptUntilShowPic();

    void setRunningScript(int scriptIdx);
    void pushRunningScript(int scriptIdx);

    void interceptPlaySoundCmd(const _2dfm::PlaySoundCmd *cmd);
    void interceptShowPicCmd(const _2dfm::ShowPic *cmd);
    void interceptColorSetCmd(const _2dfm::ColorSet *cmd);

    ax::Sprite *spriteComponent = nullptr;
    class MoveComponent *moveComponent = nullptr;

    std::vector<ScriptRunningInfo> runningStack;
    int runningScriptItemIdx = 0;
    int originalScriptIdx = 0;

    float timeWaiting = 0;
};

#endif //SCRIPTINTERCEPTORCOMPONENT_HPP
