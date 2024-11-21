//
// Created by limen on 2024/11/21.
//

#include <simdjson/simdjson.h>
#include <axmol.h>
#include "GameConfig.hpp"

const std::string configFileName("config.plist");

void GameConfig::readAndInit() {
    const auto fileUtils = ax::FileUtils::getInstance();
    const auto savePath = fileUtils->getWritablePath() + configFileName;
    if (fileUtils->isFileExist(savePath)) {
        data = fileUtils->getValueMapFromFile(savePath);
    }
}

void GameConfig::save() {
    const auto fileUtils = ax::FileUtils::getInstance();
    const auto savePath = fileUtils->getWritablePath() + configFileName;

    if (bool ret = fileUtils->writeToFile(data, savePath); !ret) {
        AXLOGW("Failed to write to config file!");
    }
}

std::string GameConfig::getGameBasePath() const {
    if (!data.contains("gameBasePath")) {
        return ".";
    }
    return data.at("gameBasePath").asString();
}
void GameConfig::setGameBasePath(std::string_view gamePath) {
    data.emplace("gameBasePath", gamePath);
}

std::string GameConfig::getKgtFileName() const {
    if (!data.contains("kgtFileName")) {
        return "GAME.kgt";
    }
    return data.at("kgtFileName").asString();
}
void GameConfig::setKgtFileName(std::string_view kgtFileName) {
    data.emplace("kgtFileName", kgtFileName);
}
