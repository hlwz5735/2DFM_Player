#pragma once

#include <string>
#include "2dfmCommon.hpp"
#include "KgtGame.hpp"

KgtGame readKgtFile(const std::string& filepath);

int get2dfmPictureRealSize(_2dfm::PictureHeader *sfh);
int get2dfmPictureSize(_2dfm::PictureHeader *sfh);

SDL_Palette *createSdlPalette(_2dfm::ColorBgra *originPalette);
