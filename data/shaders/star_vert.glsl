#version 300 es
precision mediump float;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec4 color;
layout (location = 4) in mat4 inst_model;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;

out vec4 out_color;

void main() {
    out_color = color;
    gl_Position = proj*view*inst_model*vec4(pos, 1.0);
}