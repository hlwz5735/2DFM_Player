//
// Created by limen on 2024/11/23.
//

#ifndef KGTNODE_HPP
#define KGTNODE_HPP

#include <axmol.h>

class ScriptInterceptorComponent;

class KgtNode : public ax::Node {
public:
    friend class SeamlessScrollComponent;

    virtual bool initWithVisibleHeight(int visibleHeight = 640);
    bool init() override { return initWithVisibleHeight(static_cast<int>(_director->getVisibleSize().height)); }
    void update(float delta) override;
    void destroy();

    const ax::Vec2 &getLogicPosition() const { return logicPosition; }
    void setLogicPosition(const ax::Vec2 &pos);

    ax::Sprite *getSprite() const { return spriteComp; }
    /// 主体精灵的直接父节点（残影等"跟随主体但独立绘制"的精灵挂在这里）
    ax::Node *getSpriteContainer() const { return spritePNode; }
    class MoveComponent *getMoveComp() const { return moveComp; }

    /// 残影组件（随节点一起创建，见 initWithVisibleHeight）
    class AfterimageComponent *getAfterimageComp() const { return afterimageComp; }

    ScriptInterceptorComponent *getInterceptor() const { return interceptorComp; }
    void addInterceptor(ScriptInterceptorComponent *p);

protected:
    ax::Vec2 logicPosition = ax::Vec2::ZERO;
    ax::Sprite *spriteComp = nullptr;
    ax::Node *spritePNode = nullptr;
    class MoveComponent *moveComp = nullptr;
    class ScriptInterceptorComponent *interceptorComp = nullptr;
    class AfterimageComponent *afterimageComp = nullptr;

    int visibleHeight = 0;
};

#endif //KGTNODE_HPP
