
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
    v4 color;
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


enum {
    TEXTURE_FORMAT_R8U_NORM,
    TEXTURE_FORMAT_R8S_NORM,
    TEXTURE_FORMAT_R8U_INT,
    TEXTURE_FORMAT_R8S_INT,
    TEXTURE_FORMAT_R16U_INT,
    TEXTURE_FORMAT_R16S_INT,
    TEXTURE_FORMAT_R16_FLOAT,
    TEXTURE_FORMAT_RG8U_NORM,
    TEXTURE_FORMAT_RG8S_NORM,
    TEXTURE_FORMAT_RG8U_INT,
    TEXTURE_FORMAT_RG8S_INT,
    TEXTURE_FORMAT_R32U_INT,
    TEXTURE_FORMAT_R32S_INT,
    TEXTURE_FORMAT_R32_FLOAT,
    TEXTURE_FORMAT_RG16U_INT,
    TEXTURE_FORMAT_RG16S_INT,
    TEXTURE_FORMAT_RG16_FLOAT,
    TEXTURE_FORMAT_RGBA8U_NORM,
    TEXTURE_FORMAT_RGBA8S_NORM,
    TEXTURE_FORMAT_RGBA8U_INT,
    TEXTURE_FORMAT_RGBA8S_INT,
    TEXTURE_FORMAT_RG32U_INT,
    TEXTURE_FORMAT_RG32S_INT,
    TEXTURE_FORMAT_RG32_FLOAT,
    TEXTURE_FORMAT_RGBA16U_INT,
    TEXTURE_FORMAT_RGBA16S_INT,
    TEXTURE_FORMAT_RGBA16_FLOAT,
    TEXTURE_FORMAT_RGBA32U_INT,
    TEXTURE_FORMAT_RGBA32S_INT,
    TEXTURE_FORMAT_RGBA32_FLOAT
};

enum {
    TEXTURE_USAGE_COPY_SRC = (1 << 0),
    TEXTURE_USAGE_COPY_DST = (1 << 1),
    TEXTURE_USAGE_TEXTURE_BINDING = (1 << 2),
    TEXTURE_USAGE_STORAGE_BINDING = (1 << 3),
    TEXTURE_USAGE_RENDER_ATTACHMENT = (1 << 4)
};

enum {
    TEXTURE_ACCESS_WRITEONLY,
    TEXTURE_ACCESS_READONLY,
    TEXTURE_ACCESS_READWRITE
};

enum {
    TEXTURE_DIM_1D,
    TEXTURE_DIM_2D,
    TEXTURE_DIM_3D,
};

enum {
    TEXTURE_VIEW_DIM_1D,
    TEXTURE_VIEW_DIM_2D,
    TEXTURE_VIEW_DIM_2DARRAY,
    TEXTURE_VIEW_DIM_CUBE,
    TEXTURE_VIEW_DIM_CUBEARRAY,
    TEXTURE_VIEW_DIM_3D
};

enum {
    SAMPLER_TYPE_FILTERING,
    SAMPLER_TYPE_NONFILTERING,
    SAMPLER_TYPE_COMPARISON
};

typedef struct texture_info_t {
    void *data;
    
    u32 dim_type;
    u32 size[3];
    
    u32 mip_level_count;
    u32 sample_count;
    
    u32 format;
    u32 usage;
} texture_info_t;


enum {
    ATTRIBUTE_VERTEX,
};

enum {
    ATTRIBUTE_FORMAT_FLOAT32x2,
    ATTRIBUTE_FORMAT_FLOAT32x3,
    ATTRIBUTE_FORMAT_FLOAT32x4,
};

enum {
    MODE_VERTEX,
    MODE_INSTANCE
};

typedef struct vertex_buffer_data_t {
    void *data;
    u64 count;
    u64 el_size;
} vertex_buffer_data_t;

typedef struct attribute_t {
    u32 loc;
    u32 type;
    u32 offset;
} attribute_t;

typedef struct vertex_buffer_layout_t {
    attribute_t *attribs;
    u32 attrib_count;
    
    vertex_buffer_data_t data;
    u32 stride, mode;
} vertex_buffer_layout_t;

enum {
    SHADER_VISIBILITY_VERTEX = (1 << 0),
    SHADER_VISIBILITY_FRAGMENT = (1 << 1),
    SHADER_VISIBILITY_COMPUTE = (1 << 2)
};

enum {
    BUFFER_FLAG_MAP_READ = (1 << 0),
    BUFFER_FLAG_MAP_WRITE = (1 << 1),
    BUFFER_FLAG_COPY_SRC = (1 << 2),
    BUFFER_FLAG_COPY_DST = (1 << 3),
    BUFFER_FLAG_INDEX = (1 << 4),
    BUFFER_FLAG_VERTEX = (1 << 5),
    BUFFER_FLAG_UNIFORM = (1 << 6),
    BUFFER_FLAG_STORAGE = (1 << 7),
    BUFFER_FLAG_INDIRECT = (1 << 8),
    BUFFER_FLAG_QUERY = (1 << 9)
};

typedef struct buffer_info_t {
    u64 size;
    u32 usage;
} buffer_info_t;

enum {
    BUFFER_TYPE_UNIFORM,
    BUFFER_TYPE_STORAGE,
    BUFFER_TYPE_READ_ONLY_STORAGE,
};

// TODO(ajeej): If you have a dynamic offset
// with arrays, the array has to be 16 bytes
// aligned when given, you can split the 
// count to represent two different counts in
// those different situations but im not sure
// how or if i want to fix this.

typedef struct sampler_bind_layout_t {
    u32 id;
    u32 type;
} sampler_bind_layout_t;

typedef struct storage_texture_bind_layout_t {
    u32 id;
    u32 access;
} storage_texture_bind_layout_t;

typedef struct texture_bind_layout_t {
    u32 id;
} texture_bind_layout_t;

typedef struct buffer_bind_layout_t {
    void *data;
    
    u32 buffer_id;
    u32 buffer_type;
    
    u32 offset;
    u32 size;
    u32 aligned_size;
    
    u32 has_dynamic_offset;
    
    u32 count;
    u32 stride;
    u32 id_offset;
} buffer_bind_layout_t;

enum {
    BINDING_TYPE_BUFFER,
    BINDING_TYPE_TEXTURE,
    BINDING_TYPE_STORAGE_TEXTURE,
    BINDING_TYPE_SAMPLER
};

typedef struct bind_layout_t {
    u32 type;
    
    u32 binding;
    u32 visibility;
    
    union {
        buffer_bind_layout_t buffer_layout;
        texture_bind_layout_t texture_layout;
        storage_texture_bind_layout_t storage_texture_layout;
        sampler_bind_layout_t sampler_layout;
    };
} bind_layout_t;

enum {
    BIND_GROUP_TYPE_CONSTANT,
    BIND_GROUP_TYPE_FRAME,
    BIND_GROUP_TYPE_DRAW,
};

typedef struct bind_group_layout_t {
    bind_layout_t binds[16];
    u32 count;
    
    u32 type;
} bind_group_layout_t;

enum {
    PIPELINE_TYPE_RENDER,
    PIPELINE_TYPE_COMPUTE
};

typedef struct render_pipeline_t {
    u32 shader_id;
    
    u32 *vb_layout_ids;
    u32 vb_count;
    
    u32 *bg_layout_ids;
    u32 bg_count;
    
    STACK(u32) *bg_const_ids;
    STACK(u32) *bg_frame_ids;
    STACK(u32) *bg_draw_ids;
    
    STACK(render_cmd_t) *cmds;
} render_pipeline_t;

typedef struct compute_pipeline_t {
    u32 shader_id;
    
    u32 *bg_layout_ids;
    u32 bg_count;
    
    u32 workgroup_x;
    u32 workgroup_y;
    u32 workgroup_z;
} compute_pipeline_t;

typedef struct pipeline_submission_t {
    u32 type;
    u32 id;
} pipeline_submission_t;

typedef struct renderer_t {
    
    u32 cur_pipeline;
    
    STACK(render_pipeline_t) *render_pipelines;
    STACK(compute_pipeline_t) *compute_pipelines;
    
    STACK(texture_info_t) *textures;
    STACK(buffer_info_t) *buffers;
    STACK(vertex_buffer_layout_t) *vb_layouts;
    STACK(bind_group_layout_t) *bg_layouts;
    
    STACK(pipeline_submission_t) *p_submit;
    
    STACK(vertex_t) *verts;
    STACK(u32) *indices;
    
    
    dir_light_t dir_light;
    point_light_t point_lights[POINT_LIGHT_COUNT];
    u32 point_light_count;
    
    STACK(material_t) *mats;
    STACK(mesh_info_t) *meshes;
    
    STACK(render_cmd_t) *cmds;
    
    camera_t cam;
    
    STACK(mat4) *transforms;
    
    void *api;
} renderer_t;

#endif //RENDERER_H
