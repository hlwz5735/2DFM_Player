//
// Created by limen on 2026/2/8.
//

#pragma once

#include "FileUtils.h"
#if (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32)
#include "FileUtils-win32.h"
#endif

class KgtFileUtil
#if (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32)
    : public ax::FileUtilsWin32
#else
    : public ax::FileUtils
#endif
{
public:
    KgtFileUtil();
    ~KgtFileUtil() override;

    static KgtFileUtil* create();

    bool init() override;

    void setGameRootPath(std::string gameRootPath);

    /**
     * 检查要求的文件是否存在
     * @param filename 虚拟文件路径，形似 player/0/sound/0.wav
     * @return 文件是否存在
     */
    bool isFileExist(std::string_view filename) const override;

    /**
     * 根据传入的文件路径，查找并返回文件的替换路径
     * 在我们这里，主要用于对音频的替换，我们这里使用的，是虚拟的路径
     * @param filename 虚拟文件名，形似 player/0/sound/0.wav 这种形式
     * @return
     */
    std::string fullPathForFilename(std::string_view filename) const override;

    /**
     * 根据传入的文件路径，打开虚拟文件流
     * @param filePath
     * @param mode
     * @return
     */
    std::unique_ptr<ax::IFileStream> openFileStream(std::string_view filePath, ax::IFileStream::Mode mode) const override;

protected:
    std::string gameRootPath;
};
