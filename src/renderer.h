
#ifndef RENDERER_H
#define RENDERER_H

// NOTE(ajeej): Render Structures

typedef struct color_t {
    u8 r, g, b, a;
} color_t;

typedef struct vertex_t {
    v3 pos;
    v3 norm;
    v2 uv;
    color_t color;
} vertex_t;

#define color_to_v3(r, g, b) HMM_V3(r/255.0f, g/255.0f, b/255.0f);


// NOTE(ajeej): Shader

enum {
    PRIMITIVE_POINTS = 0,
    PRIMITIVE_LINES,
    PRIMITIVE_LINE_LOOP,
    PRIMITIVE_LINE_STRIP,
    PRIMITIVE_TRIANGLES,
    PRIMITIVE_TRIANGLE_STRIP,
    PRIMITIVE_TRIANGLE_FAN,
    PRIMITIVE_QUADS,
};

// TODO(ajeej): change create_*_instance to take these inputs
enum {
    SHAPE_TRIANGLE = 0,
    SHAPE_SQUARE,
    SHAPE_SPHERE
};

enum {
#define LOC(tag, name) tag,
#include "locs.inc"
    SHADER_LOC_COUNT
#undef LOC
};

#define LOC(tag, name) global const char *tag##_NAME = #name;
#include "locs.inc"
#undef LOC

global const char *shader_loc_names[] = {
#define LOC(tag, name) #name,
#include "locs.inc"
#undef LOC
};

enum {
    ATTRIB_DATA_TYPE_BYTE = 0x1400,
    ATTRIB_DATA_TYPE_UNSIGNED_BYTE,
    ATTRIB_DATA_TYPE_SHORT,
    ATTRIB_DATA_TYPE_UNSIGNED_SHORT,
    ATTRIB_DATA_TYPE_INT,
    ATTRIB_DATA_TYPE_UNSIGNED_INT,
    ATTRIB_DATA_TYPE_FLOAT
};

enum {
    ATTRIB_TYPE_MAT4,
    ATTRIB_TYPE_MATERIAL,
};

typedef struct attrib_info_t {
    u32 count;
    u32 type;
    u32 normalize;
    u32 offset;
    
    const char *name;
} attrib_info_t;

enum {
    SHADER_ATTRIB_POS,
    SHADER_ATTRIB_NORM,
    SHADER_ATTRIB_UV,
    SHADER_ATTRIB_COLOR,
    SHADER_ATTRIB_COUNT,
};

typedef struct attrib_t {
    void *data;
    u32 count;
    u32 size;
    u32 dynamic;
    u32 update;
    u32 divisor;
    
    attrib_info_t *info;
    u32 info_count;
    
    u32 ibo;
    u32 inst_id;
    u32 shader_id;
    const char *name;
} attrib_t;

typedef struct cmd_loc_t {
    u32 idx, count;
} cmd_loc_t;

typedef struct shader_t {
    u32 id;
    i32 loc[SHADER_LOC_COUNT];
    
    STACK(u32 *) inst_ids;
    cmd_loc_t cmds;
} shader_t;


// NOTE(ajeej): Render Elements / Uniforms

typedef struct camera_t {
    v3 pos;
    quat rot;
    v3 front, up, right;
    v3 world_up;
    
    f32 speed;
    f32 sens;
    f32 orth_interp;
    f32 fov;
    f32 aspect_ratio;
    f32 n;
    f32 f;
    
    mat4 pers;
    mat4 orth;
    mat4 view;
} camera_t;

enum {
    CAMERA_UPDATE_PROJECTION = (1 << 0),
    CAMERA_UPDATE_ORIENTATION = (1 << 1),
    CAMERA_UPDATE_VIEW = (1 << 2),
};

#define CAMERA_UPDATE_ALL CAMERA_UPDATE_PROJECTION|CAMERA_UPDATE_ORIENTATION|CAMERA_UPDATE_VIEW

typedef struct dir_light_t {
    v3 dir;
    v3 ambient;
    v3 diffuse;
    v3 specular;
} dir_light_t;

typedef struct point_light_t {
    v3 pos;
    
    f32 constant;
    f32 linear;
    f32 quadratic;
    
    v3 ambient;
    v3 diffuse;
    v3 specular;
} point_light_t;

#define POINT_LIGHT_COUNT 4

typedef struct material_t {
    v3 ambient;
    v3 diffuse;
    v3 specular;
    f32 shininess;
} material_t;

#define NO_MATERIAL -1


// NOTE(ajeej): Renderer Utilities

typedef struct mesh_info_t {
    u32 indices_idx;
    u32 indices_count;
    u32 prim_type;
} mesh_info_t;

typedef struct render_cmd_t {
    u32 mesh_id;
    u32 mat_id;
    u32 trans_id;
} render_cmd_t;

// NOTE(ajeej): Instance
typedef struct instance_t {
    u32 shader_id;
    u32 mesh_id;
    u32 count;
    STACK(attrib_t) *attribs;
} instance_t;

typedef struct vertex_array_t {
    u32 vao, vbo, ebo;
    
    STACK(vertex_t) *verts;
    STACK(u32) *indices;
    
    instance_t mesh_instance;
} vertex_array_t;

typedef struct renderer_t {
    STACK(vertex_array_t) *va;
    
    u32 cur_shader;
    
    STACK(shader_t) *shaders;
    
    dir_light_t dir_light;
    point_light_t point_lights[POINT_LIGHT_COUNT];
    u32 point_light_count;
    
    STACK(material_t) *mats;
    STACK(mesh_info_t) *meshes;
    
    STACK(render_cmd_t) *cmds;
    
    camera_t cam;
    
    STACK(mat4) *transforms;
} renderer_t;

#endif //RENDERER_H
