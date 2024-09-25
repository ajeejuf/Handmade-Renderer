#version 300 es
precision mediump float;

struct dir_light_t {
    vec3 dir;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct material_t {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

in vec3 out_norm;
in vec4 out_color;
in vec3 frag_pos;

out vec4 frag_color;

uniform vec3 view_pos;
uniform dir_light_t dir_light;
uniform material_t material;

vec3 calculate_dir_light(dir_light_t light, vec3 normal, vec3 view_dir)
{
    vec3 light_dir = normalize(-light.dir);
    
    float diff = max(dot(normal, light.dir), 0.0);
    
    vec3 reflect_dir = reflect(-light.dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    
    vec3 ambient = light.ambient * material.ambient;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    
    return (ambient + diffuse + specular);
}

void main() {
    vec3 color = vec3(0.0);
    vec3 norm = normalize(out_norm);
    vec3 view_dir = normalize(view_pos - frag_pos);
    
    color = calculate_dir_light(dir_light, norm, view_dir);
    
    frag_color = vec4(color, 1.0);
}