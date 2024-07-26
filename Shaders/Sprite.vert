#version 330

uniform mat4 uM;
uniform mat4 uVP;

layout(location=0) in vec3 inPos;
layout(location=1) in vec2 inUv;

out vec2 fragUv;

void main() {
    vec4 pos = vec4(inPos, 1.0);
    fragUv = inUv;
    gl_Position = pos * uM * uVP;
}
