//
// AfterimageComponent: “残”（AFTERIMAGE）脚本项的执行者
//

#ifndef AFTERIMAGECOMPONENT_HPP
#define AFTERIMAGECOMPONENT_HPP

#include <axmol.h>

#include <cstdint>
#include <vector>

#include "2dfm/2dfmScriptItem.hpp"
#include "engine/KgtComponent.hpp"

/**
 * 残影组件。
 *
 * 语义（来自 2dfmScriptItem.hpp 中对 AfterimageCmd 的逆向注释，以及真实文件里的取值）：
 *  - afterimageGap   ：生成间隔，单位 1/100 秒
 *  - afterimageMaxCount：同时存在的残影上限；存活时间 = 个数 × 间隔
 *  - afterimageMaxCount == 0：停止生成（实测数据中这正是“关掉残影”的写法）；
 *                     已经生成的残影会继续走完淡出，不会突然消失
 *  - colorBlendType  ：残影使用的混合方式（50% 半透 / 加法 / 减法 / 指定透明度）
 *  - afterimageColorType：残影颜色随时间的变化方式
 *      NONE          不施加颜色变化（混合方式仍然生效）
 *      FIXED         整段生命周期使用固定颜色
 *      LINEAR_SMOOTH 按存活时间线性淡出（颜色与不透明度一起衰减）
 *      LINEAR_BLINK  与线性淡出相同，但每隔一帧隐藏一次（模拟半透明）
 *      RANDOM        每帧按颜色分量的方向随机取值，未指定透明度时不随机透明度
 *
 * 残影节点挂在**主体精灵所在的节点**（KgtNode::getSpriteContainer()）里，Z 序取主体精灵之下，
 * 因此一定绘制在主体之后（被主体覆盖）而不会被同层背景盖住；
 * 每帧按生成时记录的世界坐标反算局部坐标，角色移动后残影留在原地。
 */
class AfterimageComponent : public KgtComponent {
public:
    bool init() override;
    void onExit() override;
    void update(float delta) override;

    /// 应用一条“残”指令
    void applyCommand(const _2dfm::AfterimageCmd *cmd);
    /// 停止生成并清空所有残影
    void clear();

    bool isEnabled() const { return enabled; }
    size_t getGhostCount() const { return ghosts.size(); }

private:
    struct Ghost {
        ax::Sprite *sprite = nullptr;
        /// 生成时在容器内的局部坐标（= 主体精灵当时的局部坐标），保证残影起手与主体完全重合
        ax::Vec2 localPos;
        /// 生成时容器的世界坐标，之后每帧用它的位移增量把残影钉在世界原地
        ax::Vec2 containerWorldPos;
        float age = 0.f;
    };

    /// 抓取当前精灵画面生成一个残影
    void spawnGhost();
    /// 按存活进度刷新某个残影的着色与不透明度
    void refreshGhost(ax::Sprite *sprite, float progress, bool blinkVisible);
    ax::Sprite *createGhostSprite() const;

    std::vector<Ghost> ghosts;

    bool enabled = false;
    /// 距离下一个残影的计时
    float spawnTimer = 0.f;
    /// 用于 LINEAR_BLINK 的帧计数
    uint32_t frameCounter = 0;

    int maxCount = 0;
    float gapSeconds = 0.f;
    float lifeSeconds = 0.f;
    _2dfm::ColorBlendType blendType = _2dfm::ColorBlendType::NORMAL;
    _2dfm::AfterimageCmd::AfterimageColorType colorType =
        _2dfm::AfterimageCmd::AfterimageColorType::NONE;
    /// 已换算到着色器空间（[-1,1]）的颜色叠加量
    float colorR = 0.f;
    float colorG = 0.f;
    float colorB = 0.f;
    /// alpha 字节换算出的不透明度；0 表示未指定
    uint8_t alphaOpacity = 0;
};

#endif //AFTERIMAGECOMPONENT_HPP
