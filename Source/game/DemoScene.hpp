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
    enum class DemoType {
        OPENING,
        TITLE,
        CHAR_SEL,
        STORY,
        CONTINUE,
    };
    bool initWithFile(std::string_view filePath, DemoType demoType = DemoType::STORY);
    void update(float deltaTime) override;
    void onExit() override;
private:
    bool initTitle();
    void updateOpening();
    void updateTitle();
    void updateCharacterSelection();
    void updateStory();

    void titleNextMode();
    void titlePrevMode();

    std::vector<ax::Node *> scriptNodes;
    KgtDemo *demo = nullptr;
    DemoType demoType = DemoType::OPENING;
    class KgtNode *cursorNode = nullptr;

    int cursorIdx = -1;
};

#endif //DEMOSCENE_HPP
