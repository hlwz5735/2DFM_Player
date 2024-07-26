//
// Created by 厉猛 on 2024-07-26.
//

#ifndef INC_2DFM_PLAYER_BASE_HPP
#define INC_2DFM_PLAYER_BASE_HPP

#include <cmath>

bool nearZero(float val, float epsilon = 0.001f) {
    return abs(val) <= epsilon;
}

#endif //INC_2DFM_PLAYER_BASE_HPP
