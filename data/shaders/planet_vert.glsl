#version 300 es
precision mediump float;

struct material_t {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec4 color;
layout (location = 4) in mat4 inst_model;

layout (location = 8) in vec3 inst_mat_ambient;
layout (location = 9) in vec3 inst_mat_diffuse;
layout (location = 10) in vec3 inst_mat_specular;
layout (location = 11) in float inst_mat_shininess;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;

out vec3 frag_pos;
out vec3 out_norm;
out vec4 out_color;

out material_t mat;

void main() {
    
    frag_pos = vec3(inst_model*vec4(pos, 1.0));
    out_norm = mat3(transpose(inverse(inst_model))) * norm;
    out_color = color;
    
    mat.ambient = inst_mat_ambient;
    mat.diffuse = inst_mat_diffuse;
    mat.specular = inst_mat_specular;
    mat.shininess = inst_mat_shininess;
    
    gl_Position = proj*view*vec4(frag_pos, 1.0);
}