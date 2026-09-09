#pragma once

#include <axmol.h>

namespace ax {
class Sprite;
}
class CommonResource;

class TestPictureScene : public ax::Scene {
public:
    bool init() override;

    void update(float delta) override;

    // Keyboard
    void onKeyPressed(ax::EventKeyboard::KeyCode code, ax::Event *event);
    void onKeyReleased(ax::EventKeyboard::KeyCode code, ax::Event *event);
private:
    ax::Sprite *sprite;
    CommonResource *cr = nullptr;
    int picNo = 0;
    ax::Label *infoLabel = nullptr;

    /// 图片数组中第一个有实际纹理的槽位；全部为空返回 0
    int firstValidPic() const;
    /// 从 current 沿 dir(+1/-1) 方向循环查找下一个有实际纹理的槽位；找不到返回 current
    int nextValidPic(int current, int dir) const;
};
