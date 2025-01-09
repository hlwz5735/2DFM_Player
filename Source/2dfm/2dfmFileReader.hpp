#pragma once

#include "2dfmCommon.hpp"
#include "KgtGame.hpp"
#include "KgtDemo.hpp"
#include "KgtStage.hpp"
#include "KgtPlayer.hpp"
#include <string>

KgtGame *readKgtFile(const std::string &filepath);
KgtDemo *readDemoFile(std::string_view filepath);
KgtStage *readStageFile(const std::string &filepath);
KgtPlayer *readPlayerFile(const std::string &filepath);

void createTexturesForCommonResource(CommonResource *cr, int paletteNo = 0);

/// 读取公共资源部分
/// 注意offset一方面会作为当前起始点，一方面会作为读取完毕后的偏移量传出
_2dfm::CommonResourcePart readCommonResourcePart(long *offset, FILE *file);

int get2dfmPictureRealSize(_2dfm::PictureHeader *sfh);
int get2dfmPictureSize(_2dfm::PictureHeader *sfh);

class KgtPalette *createPalette(_2dfm::ColorBgra *originPalette, bool isPrivate = false);
