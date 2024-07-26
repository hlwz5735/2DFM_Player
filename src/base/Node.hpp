//
// Created by 厉猛 on 2024-07-16.
//

#ifndef INC_2DFM_PLAYER_NODE_HPP
#define INC_2DFM_PLAYER_NODE_HPP

#include <vector>
#include <typeinfo>
#include "Component.hpp"
#include "../math/Vector.hpp"

class Game;

class Node {
public:
    friend class Game;
    enum State { ACTIVE, PAUSED, DEAD };

    Node(Game *game);
    virtual ~Node();

    virtual void update(float deltaTime) {}

    const Vector2 &getPosition() const { return position; }
    void setPosition(const Vector2 &pos) { position = pos; }
    State getState() const { return state; }
    void setState(State state) { this->state = state; }
    Game *getGame() const { return game; }

    void addComponent(Component *component);
    void removeComponent(Component *component);

    template<typename T>
    T *getComponent() {
        auto iter = components.begin();
        while(iter != components.end()) {
            if (typeid(**iter) == typeid(T)) {
                return dynamic_cast<T *>(*iter);
            }
            ++iter;
        }
        return nullptr;
    }
protected:
    void __update(float deltaTime);
    void updateComponents(float deltaTime);
private:
    State state;

    Vector2 position;

    std::vector<Node *> children;
    std::vector<Component *> components;
    Game *game;
};

#endif //INC_2DFM_PLAYER_NODE_HPP
