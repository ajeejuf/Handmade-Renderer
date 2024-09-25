#version 300 es
precision mediump float;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec4 color;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;

out vec3 frag_pos;
out vec3 out_norm;
out vec4 out_color;

void main() {
    
    frag_pos = vec3(model * vec4(pos, 1.0));
    out_norm = mat3(transpose(inverse(model))) * norm;
    out_color = color;
    
    gl_Position = proj*view*vec4(frag_pos, 1.0);
}