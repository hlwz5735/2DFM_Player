//
// Created by 厉猛 on 2024-07-26.
//

#ifndef INC_2DFM_PLAYER_BASE_HPP
#define INC_2DFM_PLAYER_BASE_HPP

#include <cmath>
#include <random>

bool nearZero(float val, float epsilon = 0.001f) {
    return abs(val) <= epsilon;
}

bool doRandom(int max, int compareValue) {
    // 静态变量，避免每次调用时重新创建
    static std::random_device rd;
    // 使用 rd() 初始化随机数引擎
    static std::mt19937 gen(rd());

    // 指定生成的区间
    std::uniform_int_distribution<> dist(0, max);

    // 生成随机数
    auto val = dist(gen);
    return val > compareValue;
}

#endif //INC_2DFM_PLAYER_BASE_HPP
