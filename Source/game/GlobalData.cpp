//
// Created by limen on 2024/8/1.
//

#include "GlobalData.hpp"

GlobalData *GlobalData::INSTANCE = nullptr;

GlobalData::GlobalData() {}
GlobalData::~GlobalData() {}

GlobalData *GlobalData::getInstance() {
    if (!INSTANCE) {
        INSTANCE = new GlobalData;
    }
    return INSTANCE;
}
