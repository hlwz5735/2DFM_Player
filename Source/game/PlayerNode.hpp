//
// Created by limen on 2025/1/10.
//

#ifndef PLAYERNODE_HPP
#define PLAYERNODE_HPP

#include <axmol.h>

#include "engine/KgtNode.hpp"

class PlayerNode : public KgtNode {
public:
    bool initWithVisibleHeight(int visibleHeight = 640) override;
};



#endif //PLAYERNODE_HPP
