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

private:
    KgtPlayer *playerData = nullptr;
};


#endif //PLAYERSCRIPTINTERCEPTOR_HPP
