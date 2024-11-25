//
// Created by limen on 2024/11/23.
//

#ifndef KGTNODE_HPP
#define KGTNODE_HPP

#include <axmol.h>

class KgtNode : public ax::Node {
public:
    bool init() override;
    void setLogicPosition(const ax::Vec2 &pos);
private:
    int visibleHeight = 0;
};



#endif //KGTNODE_HPP
