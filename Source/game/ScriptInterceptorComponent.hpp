//
// Created by limen on 2024/11/22.
//

#ifndef SCRIPTINTERCEPTORCOMPONENT_HPP
#define SCRIPTINTERCEPTORCOMPONENT_HPP

#include <axmol.h>

struct CommonResource;
namespace _2dfm {
    struct PlaySoundCmd;
    struct ShowPic;
    struct ColorSet;
}

class ScriptInterceptorComponent : public ax::Component {
public:
    bool init() override;
    void onAdd() override;

    void setRunningScript(int scriptIdx);

    virtual const CommonResource *getCommonResource() const = 0;
protected:
    bool hasNoShowPicItem() const;
    _2dfm::ShowPic *interceptScriptUntilShowPic();

    void interceptPlaySoundCmd(const _2dfm::PlaySoundCmd *cmd);
    void interceptShowPicCmd(const _2dfm::ShowPic *cmd);
    void interceptColorSetCmd(const _2dfm::ColorSet *cmd);

    ax::Sprite *spriteComponent = nullptr;
    class MoveComponent *moveComponent = nullptr;

    int runningScriptItemIdx = 0;
    int startIdx = 0;
    int endIdx = 0;

    float timeWaiting = 0;
};

#endif //SCRIPTINTERCEPTORCOMPONENT_HPP
