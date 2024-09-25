#version 300 es
precision mediump float;

struct dir_light_t {
    vec3 dir;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct point_light_t {
    vec3 pos;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NUM_POINT_LIGHTS 4

struct material_t {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

in vec3 out_norm;
in vec4 out_color;
in vec3 frag_pos;
in material_t mat;

out vec4 frag_color;

uniform vec3 view_pos;
uniform dir_light_t dir_light;
uniform point_light_t point_lights[NUM_POINT_LIGHTS];

vec3 calculate_dir_light(dir_light_t light, vec3 normal, vec3 view_dir)
{
    vec3 light_dir = normalize(-light.dir);
    
    float diff = max(dot(normal, light.dir), 0.0);
    
    vec3 reflect_dir = reflect(-light.dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), mat.shininess);
    
    vec3 ambient = light.ambient * mat.ambient;
    vec3 diffuse = light.diffuse * diff * mat.diffuse;
    vec3 specular = light.specular * spec * mat.specular;
    
    return (ambient + diffuse + specular);
}

vec3 calculate_point_light(point_light_t light, vec3 normal, vec3 frag_pos, vec3 view_dir)
{
    vec3 light_dir = normalize(light.pos - frag_pos);
    float diff = max(dot(normal, light_dir), 0.0);
    
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 10.0);
    
    float dist = length(light.pos - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist*dist));
    
    vec3 ambient = light.ambient * mat.ambient;
    vec3 diffuse = light.diffuse * diff * mat.diffuse;
    vec3 specular = light.specular * spec * mat.specular;
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return (ambient + diffuse + specular);
}

void main() {
    vec3 norm = normalize(out_norm);
    vec3 view_dir = normalize(view_pos - frag_pos);
    
    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    
    result += calculate_dir_light(dir_light, norm, view_dir);
    
    /*for (int i = 0; i < 1; i++)
        result += calculate_point_light(point_lights[i], norm, frag_pos, view_dir);*/
    
    frag_color = vec4(result, 1.0);
}