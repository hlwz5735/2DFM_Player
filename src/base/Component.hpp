//
// Created by 厉猛 on 2024-07-16.
//

#ifndef INC_2DFM_PLAYER_COMPONENT_HPP
#define INC_2DFM_PLAYER_COMPONENT_HPP

class Node;

class Component {
public:
    Component(Node *owner, int updateOrder = 100);
    virtual ~Component();

    virtual void update(float deltaTime);

    int getUpdateOrder() const { return updateOrder; }

protected:
    Node *owner;
    int updateOrder;
};


#endif //INC_2DFM_PLAYER_COMPONENT_HPP
