#pragma once

#include <string>
#include "2dfmCommon.hpp"
#include "KgtGame.hpp"
#include "KgtDemo.hpp"

KgtGame *readKgtFile(const std::string &filepath);
KgtDemo *readDemoFile(const std::string &filepath);

/// ��ȡ������Դ����
/// ע��offsetһ�������Ϊ��ǰ��ʼ�㣬һ�������Ϊ��ȡ��Ϻ��ƫ��������
_2dfm::CommonResourcePart readCommonResourcePart(long *offset, FILE *file);

int get2dfmPictureRealSize(_2dfm::PictureHeader *sfh);
int get2dfmPictureSize(_2dfm::PictureHeader *sfh);

SDL_Palette *createSdlPalette(_2dfm::ColorBgra *originPalette);
