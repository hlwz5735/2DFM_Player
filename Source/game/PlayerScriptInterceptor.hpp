//
// Created by limen on 2025/1/9.
//

#ifndef PLAYERSCRIPTINTERCEPTOR_HPP
#define PLAYERSCRIPTINTERCEPTOR_HPP

#include <axmol.h>

#include "2dfm/KgtPlayer.hpp"
#include "ScriptInterceptorComponent.hpp"

class PlayerScriptInterceptor : public ScriptInterceptorComponent {
public:
    void update(float delta) override;

    const CommonResource *getCommonResource() const override { return playerData; }

    void setPlayerData(KgtPlayer *playerData);
    class PlayerNode *getPlayerNode() const;
protected:
    void interceptObjectCmd(const _2dfm::ObjectCmd *cmd) const override;
private:
    KgtPlayer *playerData = nullptr;
    mutable class PlayerNode *playerNode = nullptr;
};


#endif //PLAYERSCRIPTINTERCEPTOR_HPP
