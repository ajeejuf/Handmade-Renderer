
#ifndef ENTITY_H
#define ENTITY_H

typedef struct transform_info_t {
    v3 pos;
    v3 rot;
    v3 scale;
} transform_info_t;

typedef struct physic_comp_t {
    v3 a, v;
} physic_comp_t;

typedef struct entity_t {
    u32 mesh_id;
    u32 trans_id;
    u32 mat_id;
    
    u32 physic_id;
} entity_t;

typedef struct text_t {
    u32 font_id;
    u32 ch_count;
    u32 trans_id;
    
    u32 update;
    
    f32 size;
    v3 pos;
} text_t;

#endif //ENTITY_H
