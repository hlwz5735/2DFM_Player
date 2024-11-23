#pragma once
#include <axmol.h>
#include <string>

#include "engine/Singleton.hpp"

class GameConfig : public Singleton<GameConfig> {
public:
    void readAndInit();
    void save();

    std::string getGameBasePath() const;
    void setGameBasePath(std::string_view gamePath);
    std::string getKgtFileName() const;
    void setKgtFileName(std::string_view kgtFileName);
private:
    ax::ValueMap data;
};
