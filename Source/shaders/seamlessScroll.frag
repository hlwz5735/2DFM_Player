#version 310 es
precision highp float;
precision highp int;

layout(location = COLOR0) in vec4 v_color;
layout(location = TEXCOORD0) in vec2 v_texCoord;

layout(binding = 0) uniform sampler2D u_tex0;

layout(location = SV_Target0) out vec4 FragColor;

layout(std140) uniform fs_ub {
    vec3 _2dfmColor;
    vec2 u_tileScale;
};

void main()
{
    vec2 tiledUV = fract(v_texCoord * u_tileScale);
    vec4 texColor = texture(u_tex0, tiledUV);
    texColor.xyz += _2dfmColor.xyz;
    texColor.a *= v_color.a;
    FragColor = texColor;
}
