//
// Created by limen on 2025/1/9.
//

#ifndef GAMEDEMO_HPP
#define GAMEDEMO_HPP

#include "2dfm/KgtDemo.hpp"
#include <axmol.h>
#include <memory>

class GameDemo : public ax::Node {
public:
    bool init() override;
    void onExit() override;

    void load(int demoNo);
    void unload();
private:
    std::vector<ax::Node *> scriptNodes;
    std::shared_ptr<KgtDemo> demo;
};

#endif //GAMEDEMO_HPP
