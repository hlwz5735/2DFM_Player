//
// Created by limen on 2026/2/8.
//

#include "KgtFileUtil.hpp"
#include "InMemFileStream.hpp"
#include "game/ResourcePool.hpp"

#include <charconv>
#include <vector>
#include <string_view>

#include "2dfm/CommonResource.hpp"
#include "SoundClip.hpp"

// ─── 内部辅助 ────────────────────────────────────────────────────────────────
namespace {

// 已知的虚拟路径前缀 (type segment)
constexpr std::string_view kKnownTypes[] = {"kgt", "player", "demo", "stage"};
// 已知的资源类型 segment
constexpr std::string_view kKnownResTypes[] = {"pic", "shared_palette", "sound"};

enum class FileType { KGT, PLAYER, DEMO, STAGE };
enum class ResourceType { PIC, SHARED_PALETTE, SOUND };

struct FilePathEntry {
    FileType type;
    int fileIndex;
    ResourceType resourceType;
    int resourceIndex;
};

/// 尝试将 string_view 解析为非负整数，失败返回 false
bool tryParseInt(std::string_view sv, int& out) {
    if (sv.empty()) return false;
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), out);
    return ec == std::errc{} && ptr == sv.data() + sv.size() && out >= 0;
}

/// 找到第 n 个 '/' 的位置，找不到返回 npos
size_t findNthSlash(std::string_view path, int n) {
    size_t pos = 0;
    for (int i = 0; i < n; ++i) {
        pos = path.find('/', pos);
        if (pos == std::string_view::npos) return pos;
        ++pos;
    }
    return pos - 1;
}

/**
 * 尝试解析虚拟路径，成功返回 true 并填充 entry。
 * 格式: <type>/<fileIndex>/<resourceType>/<resourceIndex>.<ext>
 */
bool tryParseFileEntry(std::string_view filename, FilePathEntry& entry) {
    // 快速检查：至少需要 3 个 '/'
    auto s1 = filename.find('/');
    if (s1 == std::string_view::npos) return false;
    auto s2 = filename.find('/', s1 + 1);
    if (s2 == std::string_view::npos) return false;
    auto s3 = filename.find('/', s2 + 1);
    if (s3 == std::string_view::npos) return false;
    // 不能再有第4个 '/'
    if (filename.find('/', s3 + 1) != std::string_view::npos) return false;

    auto typeStr    = filename.substr(0, s1);
    auto fileIdxStr = filename.substr(s1 + 1, s2 - s1 - 1);
    auto resTypeStr = filename.substr(s2 + 1, s3 - s2 - 1);
    auto baseName   = filename.substr(s3 + 1);

    // 1. 解析文件类型
    if (typeStr == "kgt")         entry.type = FileType::KGT;
    else if (typeStr == "player") entry.type = FileType::PLAYER;
    else if (typeStr == "demo")   entry.type = FileType::DEMO;
    else if (typeStr == "stage")  entry.type = FileType::STAGE;
    else return false;

    // 2. 解析文件序号
    if (!tryParseInt(fileIdxStr, entry.fileIndex)) return false;

    // 3. 解析资源类型
    if (resTypeStr == "pic")              entry.resourceType = ResourceType::PIC;
    else if (resTypeStr == "shared_palette") entry.resourceType = ResourceType::SHARED_PALETTE;
    else if (resTypeStr == "sound")       entry.resourceType = ResourceType::SOUND;
    else return false;

    // 4. 解析资源序号 (去掉扩展名)
    auto dotPos = baseName.find('.');
    auto resIdxStr = (dotPos != std::string_view::npos) ? baseName.substr(0, dotPos) : baseName;
    if (!tryParseInt(resIdxStr, entry.resourceIndex)) return false;

    return true;
}

} // anonymous namespace

// ─── 公共接口 ────────────────────────────────────────────────────────────────

bool KgtFileUtil::isKgtVirtualPath(std::string_view path) {
    FilePathEntry dummy;
    return tryParseFileEntry(path, dummy);
}

KgtFileUtil* KgtFileUtil::create() {
    auto* fileUtils = new (std::nothrow) KgtFileUtil();
    if (fileUtils && fileUtils->init()) {
        return fileUtils;
    }
    delete fileUtils;
    return nullptr;
}

bool KgtFileUtil::init() {
    return Super::init();
}

void KgtFileUtil::setGameRootPath(std::string gameRootPath) {
    purgeCachedEntries();
    this->gameRootPath = std::move(gameRootPath);
}

// ─── 虚拟文件劫持 ─────────────────────────────────────────────────────────────

bool KgtFileUtil::isFileExist(std::string_view filename) const {
    FilePathEntry entry;
    if (!tryParseFileEntry(filename, entry)) {
        return Super::isFileExist(filename);
    }

    // 虚拟路径：通过 ResourcePool 检查资源是否存在
    const auto& pool = ResourcePool::getInstance();
    ResourcePool::FileType ft;
    switch (entry.type) {
    case FileType::KGT:    ft = ResourcePool::FileType::KGT; break;
    case FileType::PLAYER: ft = ResourcePool::FileType::PLAYER; break;
    case FileType::DEMO:   ft = ResourcePool::FileType::DEMO; break;
    case FileType::STAGE:  ft = ResourcePool::FileType::STAGE; break;
    }
    return pool.findResource(ft, entry.fileIndex) != nullptr;
}

std::string KgtFileUtil::fullPathForFilename(std::string_view filename) const {
    if (filename.empty()) return "";

    // 虚拟路径原样返回（不需要文件系统解析）
    if (isKgtVirtualPath(filename)) {
        return std::string(filename);
    }
    // 非虚拟路径 → 透传父类
    return Super::fullPathForFilename(filename);
}

std::unique_ptr<ax::IFileStream> KgtFileUtil::openFileStream(
    std::string_view filePath, ax::IFileStream::Mode mode
) const {
    // 非 READ 模式 或 非虚拟路径 → 直接透传父类
    FilePathEntry entry;
    if (mode != ax::IFileStream::Mode::READ || !tryParseFileEntry(filePath, entry)) {
        return Super::openFileStream(filePath, mode);
    }

    // 虚拟路径 READ：目前仅支持 SOUND 资源
    if (entry.resourceType != ResourceType::SOUND) {
        return nullptr;
    }

    // 通过 ResourcePool 查找资源
    ResourcePool::FileType ft;
    switch (entry.type) {
    case FileType::KGT:    ft = ResourcePool::FileType::KGT; break;
    case FileType::PLAYER: ft = ResourcePool::FileType::PLAYER; break;
    case FileType::DEMO:   ft = ResourcePool::FileType::DEMO; break;
    case FileType::STAGE:  ft = ResourcePool::FileType::STAGE; break;
    }

    const auto& pool = ResourcePool::getInstance();
    auto* cr = pool.findResource(ft, entry.fileIndex);
    if (!cr) return nullptr;

    if (entry.resourceIndex < 0 || entry.resourceIndex >= (int)cr->sounds.size()) {
        return nullptr;
    }

    auto* sc = cr->sounds[entry.resourceIndex]->getRawSoundPtr();
    if (!sc) return nullptr;

    std::vector<byte> dataVec(sc->header.size);
    std::memcpy(dataVec.data(), sc->content, sc->header.size);

    auto stream = std::make_unique<InMemFileStream>();
    stream->setBuffer(std::move(dataVec));
    return stream;
}
