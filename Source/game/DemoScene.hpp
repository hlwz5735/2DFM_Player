//
// Created by limen on 2024/11/21.
//

#ifndef DEMOSCENE_HPP
#define DEMOSCENE_HPP

#include <axmol.h>
#include <vector>
#include "2dfm/KgtDemo.hpp"

class DemoScriptInterceptor;
class DemoScene : public ax::Scene {
public:
    bool initWithFile(std::string_view filePath);
private:
    std::vector<ax::Node *> scriptNodes;
    KgtDemo *demo = nullptr;
};

#endif //DEMOSCENE_HPP
