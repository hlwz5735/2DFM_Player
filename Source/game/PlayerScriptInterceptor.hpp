//
// Created by limen on 2025/1/9.
//

#ifndef PLAYERSCRIPTINTERCEPTOR_HPP
#define PLAYERSCRIPTINTERCEPTOR_HPP

#include <axmol.h>
#include <memory>

#include "2dfm/KgtPlayer.hpp"
#include "ScriptInterceptorComponent.hpp"

class PlayerScriptInterceptor : public ScriptInterceptorComponent {
public:
    void update(float delta) override;

    const CommonResource *getCommonResource() const override { return playerData.get(); }

    void setPlayerData(std::shared_ptr<KgtPlayer> playerData);
    class PlayerNode *getPlayerNode() const;
protected:
    void interceptObjectCmd(const _2dfm::ObjectCmd *cmd) const override;
private:
    std::shared_ptr<KgtPlayer> playerData;
    mutable class PlayerNode *playerNode = nullptr;
};


#endif //PLAYERSCRIPTINTERCEPTOR_HPP
