//
// Created by limen on 2026/2/8.
//


#include "KgtFileUtil.hpp"
#include "InMemFileStream.hpp"
#include "game/GameManager.hpp"

#include <vector>
#include <string_view>

#include "2dfm/KgtGame.hpp"
#include "2dfm/KgtStage.hpp"
#include "SoundClip.hpp"

namespace {
int toInt(std::string_view sv) {
    if (sv.empty()) {
        throw std::invalid_argument("empty string");
    }
    size_t pos = 0;
    try {
        int val = std::stoi(std::string(sv), &pos);
        if (pos != sv.size()) {
            throw std::invalid_argument("non-numeric suffix");
        }
        return val;
    } catch (...) {
        throw std::invalid_argument("invalid integer: " + std::string(sv));
    }
}

// 将 string_view 按 '/' 分割为多个 token
std::vector<std::string_view> splitPath(std::string_view path, std::string delimiter = "/") {
    std::vector<std::string_view> tokens;
    size_t start = 0;
    size_t end = 0;

    while ((end = path.find_first_of(delimiter, start)) != std::string_view::npos) {
        tokens.push_back(path.substr(start, end - start));
        start = end + 1;
    }
    // 添加最后一段（即使为空）
    tokens.push_back(path.substr(start));

    return tokens;
}

enum class FileType {
    KGT, PLAYER, DEMO, STAGE,
};
enum class ResourceType {
    PIC,
    SHARED_PALETTE,
    SOUND,
};

struct FilePathEntry {
    FileType type;
    int fileIndex;
    ResourceType resourceType;
    int resourceIndex;
};

/**
 * kgt/0/sound/0.wav
 * player/0/sound/13.wav
 */
FilePathEntry parseFileEntry(std::string_view filename) {
    auto parts = splitPath(filename);

    // 格式必须是: type / fileIndex / resourceType / basename.ext
    if (parts.size() != 4) {
        throw std::invalid_argument("invalid path format: expected 4 components");
    }

    const auto& [typeStr, fileIdxStr, resTypeStr, basenameExt] =
        std::tie(parts[0], parts[1], parts[2], parts[3]);

    FilePathEntry entry;

    // 1. 解析文件类型
    if (typeStr == "player") {
        entry.type = FileType::PLAYER;
    } else if (typeStr == "demo") {
        entry.type = FileType::DEMO;
    } else if (typeStr == "stage") {
        entry.type = FileType::STAGE;
    } else if (typeStr == "kgt") {
        entry.type = FileType::KGT;
    } else {
        throw std::invalid_argument("unknown file type: " + std::string(typeStr));
    }

    // 2. 解析文件序号
    entry.fileIndex = toInt(fileIdxStr);

    // 3. 解析资源类型
    if (resTypeStr == "pic") {
        entry.resourceType = ResourceType::PIC;
    } else if (resTypeStr == "shared_palette") {
        entry.resourceType = ResourceType::SHARED_PALETTE;
    } else if (resTypeStr == "sound") {
        entry.resourceType = ResourceType::SOUND;
    } else {
        throw std::invalid_argument("unknown resource type: " + std::string(resTypeStr));
    }

    // 4. 解析资源序号（去掉扩展名）
    auto dotPos = basenameExt.find('.');
    std::string_view resIdxStr = (dotPos != std::string_view::npos)
        ? basenameExt.substr(0, dotPos)
        : basenameExt;

    if (resIdxStr.empty()) {
        throw std::invalid_argument("empty resource index");
    }
    entry.resourceIndex = toInt(resIdxStr);

    return entry;
}
}

KgtFileUtil::KgtFileUtil() {

}

KgtFileUtil::~KgtFileUtil() {

}

KgtFileUtil *KgtFileUtil::create() {
    auto* fileUtils = new (std::nothrow) KgtFileUtil();
    fileUtils->init();
    return fileUtils;
}
bool KgtFileUtil::init() {
#if (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32)
    if (!FileUtilsWin32::init()) {
#endif
        return false;
    }

    return true;
}

void KgtFileUtil::setGameRootPath(std::string gameRootPath) {
    purgeCachedEntries();

    this->gameRootPath = gameRootPath;
}

bool KgtFileUtil::isFileExist(std::string_view filename) const {
    try {
        auto entry = parseFileEntry(filename);
        const auto &gameManager = GameManager::getInstance();
        const auto kgt = gameManager.getKgtGame();
        if (kgt == nullptr) {
            return false;
        }
        if (entry.type == FileType::KGT) {
            return kgt != nullptr;
        }
        if (entry.type == FileType::DEMO) {
            return entry.resourceIndex >= 0 && entry.resourceIndex < kgt->demoNames.size();
        }
        if (entry.type == FileType::PLAYER) {
            return entry.resourceIndex >= 0 && entry.resourceIndex < kgt->playerNames.size();
        }
        return entry.resourceIndex >= 0 && entry.resourceIndex < kgt->stageNames.size();
    } catch (...) {
        AXLOG("In catch: [%.*s]\n", (int)filename.size(), filename.data());
        return FileUtilsWin32::isFileExist(filename);
    }
}

std::string KgtFileUtil::fullPathForFilename(std::string_view filename) const {
    // 没啥可转换的，我这里，原样输出
    if (filename.empty()) {
        return "";
    }
    try {
        parseFileEntry(filename);
        return std::string(filename);
    } catch (...) {
        return FileUtilsWin32::fullPathForFilename(filename);
    }

}

std::unique_ptr<ax::IFileStream> KgtFileUtil::openFileStream(
    std::string_view filePath, ax::IFileStream::Mode mode
) const {
    switch (mode)
    {
    case ax::IFileStream::Mode::READ:
        // 只支持只读模式
        break;
    case ax::IFileStream::Mode::WRITE:
    case ax::IFileStream::Mode::APPEND:
    case ax::IFileStream::Mode::OVERLAPPED:
    default:
        AXASSERT(false, "UNKNOWN FileStream::Mode");
        return nullptr;
    }

    try {
        auto entry = parseFileEntry(filePath);
        InMemFileStream inMemFileStream;

        if (entry.resourceType == ResourceType::SOUND) {
            _2dfm::Sound *sc;
            if (entry.type == FileType::KGT) {
                auto kgtGame = GameManager::getInstance().getKgtGame();
                sc = kgtGame->sounds[entry.resourceIndex]->getRawSoundPtr();
            } else if (entry.type == FileType::STAGE) {
                auto kgtStage = GameManager::getInstance().getKgtStage();
                sc = kgtStage->sounds[entry.resourceIndex]->getRawSoundPtr();
            } else {
                return nullptr;
            }
            auto dataVec = std::vector<byte>(sc->header.size);
            std::memcpy(dataVec.data(), sc->content, sc->header.size);
            inMemFileStream.setBuffer(dataVec);
            return std::make_unique<InMemFileStream>(std::move(inMemFileStream));
        }

        return nullptr;
    } catch (...) {
        return FileUtilsWin32::openFileStream(filePath, mode);
    }
}
