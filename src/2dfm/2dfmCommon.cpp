//
// Created by 厉猛 on 2024-07-17.
//
#include <format>
#include "2dfmCommon.hpp"

using namespace _2dfm;

std::string ColorBgra::toString() {
    return std::format("0x{:2X}{:2X}{:2X}{:2X}",
        (int) channel.red,
        (int) channel.green,
        (int) channel.blue,
        channel.alpha == static_cast<byte>(0) ? 0 : 255);
}
