#version 450 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec2 vtxUv;
layout(location = 1) out vec4 vtxColor;

vec2 toNDC(vec2 coords) {
    return vec2(coords.x * 2.0 - 1.0, -(coords.y * 2.0 - 1.0));
}

void main() {
    gl_Position = vec4(toNDC(inPos), 0.0, 1.0);
    vtxUv = inUv;
    vtxColor = inColor;
}