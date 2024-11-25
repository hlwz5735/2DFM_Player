//
// Created by limen on 2024/11/25.
//

#ifndef SEAMLESSSCROLLCOMPONENT_HPP
#define SEAMLESSSCROLLCOMPONENT_HPP

#include <axmol.h>

class SeamlessScrollComponent : public ax::Component {
public:
    // bool init() override;
    void onAdd() override;

    void update(float delta) override;

    void setHoriSeamless(bool v) { horiSeamless = v; }
    void setVertSeamless(bool v) { vertSeamless = v; }

    void updateSprite();

private:
    int seedWidth = 0;
    int seedHeight = 0;
    bool horiSeamless = false;
    bool vertSeamless = false;
    ax::Sprite *seedSprite = nullptr;
    ax::Node *pNode = nullptr;
};

#endif //SEAMLESSSCROLLCOMPONENT_HPP
