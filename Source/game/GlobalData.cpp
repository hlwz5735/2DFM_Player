//
// Created by limen on 2024/8/1.
//

#include "GlobalData.hpp"

GlobalData::GlobalData() {}
GlobalData::~GlobalData() {}

GlobalData *GlobalData::getInstance() {
    static GlobalData instance;
    return &instance;
}
