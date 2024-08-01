//
// Created by limen on 2024/8/1.
//

#include "FirstScene.hpp"
#include "2dfm/2dfmFileReader.hpp"
#include "GlobalData.hpp"
#include "StageTestScene.hpp"
#include <format>

bool FirstScene::init() {
    if (!Scene::init()) {
        return false;
    }
    auto &gameConfig = GlobalData::getInstance()->getGameConfig();

    auto kgtFilePath = std::format("{}/{}", gameConfig.gameBasePath, gameConfig.kgtFileName);
    auto kgt = readKgtFile(kgtFilePath);
    createTexturesForCommonResource(kgt, 0);
    GlobalData::getInstance()->setKgtGame(kgt);

    return true;
}
void FirstScene::onEnterTransitionDidFinish() {
    Scene::onEnterTransitionDidFinish();
    _director->replaceScene(ax::utils::createInstance<StageTestScene>());
}
