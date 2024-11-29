//
// Created by limen on 2024/11/28.
//

#include "KgtComponent.hpp"
#include "KgtNode.hpp"

USING_NS_AX;

KgtNode *KgtComponent::getOwner() const {
    return dynamic_cast<KgtNode *>(_owner);
}
