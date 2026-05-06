//
// Created by limen on 2026/2/8.
//

#pragma once

#include "FileUtils.h"
#if (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32)
#include "FileUtils-win32.h"
#endif

/**
 * KgtFileUtil: 对 FileUtils 的再封装
 *
 * 设计意图：仅劫持 KGT 虚拟文件路径（形如 "player/0/sound/0.wav"），
 * 所有其他路径完全透传给父类的原生实现，不做任何干预。
 */
class KgtFileUtil
#if (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32)
    : public ax::FileUtilsWin32
#else
    : public ax::FileUtils
#endif
{
#if (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32)
    using Super = ax::FileUtilsWin32;
#else
    using Super = ax::FileUtils;
#endif

public:
    KgtFileUtil() = default;
    ~KgtFileUtil() override = default;

    static KgtFileUtil* create();

    bool init() override;

    void setGameRootPath(std::string gameRootPath);

    bool isFileExist(std::string_view filename) const override;
    std::string fullPathForFilename(std::string_view filename) const override;
    std::unique_ptr<ax::IFileStream> openFileStream(std::string_view filePath, ax::IFileStream::Mode mode) const override;

    /**
     * 判断给定路径是否为 KGT 虚拟路径
     * 虚拟路径格式: <type>/<fileIndex>/<resourceType>/<resourceIndex>.<ext>
     * 例如: player/0/sound/0.wav, kgt/0/sound/13.wav, stage/1/pic/2.png
     */
    static bool isKgtVirtualPath(std::string_view path);

protected:
    std::string gameRootPath;
};
