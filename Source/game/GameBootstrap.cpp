//
// Created by limen on 2026/9/9.
//

#include "GameBootstrap.hpp"

#include "engine/Input.hpp"
#include "game/GameConfig.hpp"
#include "game/GameManager.hpp"

void GameBootstrap::initialize() {
    if (initialized) {
        return;
    }

    // 输入系统：全局键盘监听，进程内只注册一次
    Input::getInstance().init();

    // 配置：启动时从磁盘读取一次；此后运行期以内存为准（选择文件只改内存，save 才写盘）
    GameConfig::getInstance().readAndInit();

    // 自定义着色器：与 KGT 无关的渲染资产，注册一次即可
    GameManager::getInstance().init();

    initialized = true;
}
