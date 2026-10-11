//
// KgtColor: 2DFM 颜色/混合方式的公共换算
// 供“色”（ColorSetCmd）与“残”（AfterimageCmd）共用
//

#pragma once

#include "2dfm/2dfmCommon.hpp"
#include "2dfm/2dfmScriptItem.hpp"

#include <axmol.h>

/// 2DFM 颜色分量值域为 [-32, 32]，着色器里按 [-1, 1] 叠加到贴图上
inline float convertKgtRgbColorValue(byte rawByte) {
    const int8_t val = *reinterpret_cast<const char *>(&rawByte);
    return static_cast<float>(val) / 32.f;
}

/// 2DFM 的透明度是 32 减去原值，换算到 0-255
inline uint8_t convertKgtOpacityColorValue(byte rawByte) {
    const auto val = 32.f - static_cast<int>(rawByte);
    return static_cast<uint8_t>((val / 32.f) * 255);
}

/// 颜色混合方式的解析结果
struct KgtColorBlend {
    ax::BlendFunc blendFunc;
    /// 由混合方式本身决定的基准不透明度（0-255）
    uint8_t baseOpacity = 255;
};

/// 2DFM 颜色混合方式 → axmol 混合函数 + 基准不透明度
/// @param alphaOpacity ALPHA_BLEND 时使用的、由 alpha 字节换算出的不透明度
inline KgtColorBlend resolveKgtColorBlend(_2dfm::ColorBlendType type, uint8_t alphaOpacity = 255) {
    switch (type) {
    case _2dfm::ColorBlendType::TRANSPARENCY:
        return {ax::BlendFunc::ALPHA_NON_PREMULTIPLIED, 128};
    case _2dfm::ColorBlendType::ADD_BLEND:
        return {ax::BlendFunc::ADDITIVE, 255};
    case _2dfm::ColorBlendType::MINUS_BLEND:
        return {{ax::backend::BlendFactor::ONE, ax::backend::BlendFactor::ONE_MINUS_SRC_COLOR}, 255};
    case _2dfm::ColorBlendType::ALPHA_BLEND:
        return {ax::BlendFunc::ALPHA_NON_PREMULTIPLIED, alphaOpacity};
    case _2dfm::ColorBlendType::NORMAL:
    default:
        return {ax::BlendFunc::ALPHA_NON_PREMULTIPLIED, 255};
    }
}
