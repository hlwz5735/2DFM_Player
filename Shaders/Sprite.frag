#version 330
uniform sampler2D uTexture;
// 透明度值，范围从0.0（完全透明）到1.0（完全不透明）
uniform float uTransparency;
// 颜色混合
uniform vec3 uBlendColor;

in vec2 fragUv;

out vec4 outColor;

void main() {
    // 采样纹理颜色
    vec4 texColor = texture(uTexture, fragUv);

    // 将纹理颜色与透明度相乘
    vec4 colorWithAlpha = vec4(
        texColor.r + uBlendColor.r,
        texColor.g + uBlendColor.g,
        texColor.b + uBlendColor.b,
        texColor.a * uTransparency);

    // 输出最终颜色
    outColor = colorWithAlpha;
}
