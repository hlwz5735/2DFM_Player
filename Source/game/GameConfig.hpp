#pragma once
#include <string>
#include <axmol.h>

class GameConfig {
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
