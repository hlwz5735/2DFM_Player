#pragma once

#include <string>
#include "2dfmCommon.hpp"
#include "KgtGame.hpp"
#include "KgtDemo.hpp"

KgtGame *readKgtFile(const std::string &filepath);
KgtDemo *readDemoFile(const std::string &filepath);

/// 读取公共资源部分
/// 注意offset一方面会作为当前起始点，一方面会作为读取完毕后的偏移量传出
_2dfm::CommonResourcePart readCommonResourcePart(long *offset, FILE *file);

int get2dfmPictureRealSize(_2dfm::PictureHeader *sfh);
int get2dfmPictureSize(_2dfm::PictureHeader *sfh);

SDL_Palette *createSdlPalette(_2dfm::ColorBgra *originPalette);
