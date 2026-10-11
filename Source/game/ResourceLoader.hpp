//
// ResourceLoader: 2DFM 四种资源文件的唯一读取入口
//

#pragma once

#include <memory>
#include <string>

struct KgtGame;
struct KgtPlayer;
struct KgtDemo;
struct KgtStage;

/**
 * 2DFM 四种文件（KGT / DEMO / Player / Stage）在磁盘上的公共部分是
 * “文件头 + 公共资源”（脚本表、脚本项表、图片、共享调色盘、声音），
 * 之后的专有段落各不相同。
 *
 * 因此这里只保留一条读取路径：
 *
 *   打开文件 → lib2dfm_reader 解析 → 构造结果对象 → 填充公共资源 → 填充专有数据 → 释放原始结构
 *
 * 前面几步和“填充公共资源”由 ResourceLoader.cpp 中的 loadResourceFile 骨架统一完成，
 * 四种文件只需要各自提供 解析函数 / 释放函数 / 专有填充函数 这三个分叉点。
 *
 * 调用方（ResourcePool）只负责缓存与路径解析，不再重复任何格式解析逻辑。
 */
namespace kgt {
/// 读取 .kgt 主文件
std::shared_ptr<KgtGame> readGameFile(const std::string &filepath);
/// 读取 .player 角色文件
std::shared_ptr<KgtPlayer> readPlayerFile(const std::string &filepath, int playerNo);
/// 读取 .demo 影片文件
std::shared_ptr<KgtDemo> readDemoFile(const std::string &filepath, int demoNo);
/// 读取 .stage 场景文件
std::shared_ptr<KgtStage> readStageFile(const std::string &filepath, int stageNo);
} // namespace kgt
