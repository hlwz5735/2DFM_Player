//
// Created by limen on 2024/11/23.
//

#ifndef KGTSCRIPTINTERCEPTOR_HPP
#define KGTSCRIPTINTERCEPTOR_HPP

#include "2dfm/KgtGame.hpp"
#include "ScriptInterceptorComponent.hpp"

class KgtScriptInterceptor : public ScriptInterceptorComponent {
public:
    void update(float delta) override;
    void setKgtGame(const KgtGame *game) { kgtGame = game; }

    const CommonResource *getCommonResource() const override { return kgtGame; }
private:
    const KgtGame *kgtGame = nullptr;
};

#endif //KGTSCRIPTINTERCEPTOR_HPP
