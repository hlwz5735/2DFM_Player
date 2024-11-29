//
// Created by limen on 2024/11/28.
//

#ifndef KGTCOMPONENT_HPP
#define KGTCOMPONENT_HPP

#include <axmol.h>

class KgtComponent : public ax::Component {
public:
    class KgtNode *getOwner() const;
    virtual void lateUpdate(float delta) {}
};

#endif //KGTCOMPONENT_HPP
