#pragma once

#include "engine/Singleton.hpp"
#include <axmol.h>
#include <string>

class GameConfig : public Singleton<GameConfig> {
public:
    void readAndInit();
    void save();

    std::string getGameBasePath() const;
    void setGameBasePath(std::string_view gamePath);
    std::string getKgtFileName() const;
    void setKgtFileName(std::string_view kgtFileName);

    ax::Size getStageSize() const {
        return ax::Size{ static_cast<float>(stageWidth), static_cast<float>(stageHeight) };
    }

    static constexpr int stageWidth = 640 * 2;
    static constexpr int stageHeight = 480 * 2;
private:
    ax::ValueMap data;
};
