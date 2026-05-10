#pragma once

#include "2dfmCommon.hpp"
#include "KgtGame.hpp"
#include "KgtDemo.hpp"
#include "KgtStage.hpp"
#include "KgtPlayer.hpp"
#include <string>

KgtGame *readKgtFile();
KgtDemo *readDemoByNo(int demoNo);
KgtStage *readStageByNo(int stageNo);
KgtPlayer *readPlayerByNo(int playerNo);

// KgtGame *readKgtFile(const std::string &filepath);
// KgtDemo *readDemoFile(const std::string &filepath);
// KgtStage *readStageFile(const std::string &filepath);
// KgtPlayer *readPlayerFile(const std::string &filepath);

void createTexturesForCommonResource(CommonResource *cr, int paletteNo = 0);

int get2dfmPictureRealSize(_2dfm::PictureHeader *sfh);
int get2dfmPictureSize(_2dfm::PictureHeader *sfh);

class KgtPalette *createPalette(_2dfm::ColorBgra *originPalette, bool isPrivate = false);
