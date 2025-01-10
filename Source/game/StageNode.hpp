//
// Created by limen on 2025/1/10.
//

#ifndef STAGENODE_HPP
#define STAGENODE_HPP

#include "engine/KgtNode.hpp"

class StageNode : public KgtNode {
public:
    void update(float deltaTime) override;

    class ParallaxComponent *getParallaxComp() const { return parallaxComp; }
    class SeamlessScrollComponent *getSeamlessScrollComp() const { return seamlessScrollComp; }

    void addParallaxComp(class ParallaxComponent *comp);
    void addSeamlessComp(class SeamlessScrollComponent * comp);
private:
    class ParallaxComponent *parallaxComp = nullptr;
    class SeamlessScrollComponent *seamlessScrollComp = nullptr;
};

#endif //STAGENODE_HPP
