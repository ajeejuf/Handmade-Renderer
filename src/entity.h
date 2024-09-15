
#ifndef ENTITY_H
#define ENTITY_H

typedef struct transform_info_t {
    v3 pos;
    v3 rot;
    v3 scale;
} transform_info_t;

typedef struct entity_t {
    u32 mesh_id;
    u32 trans_id;
    u32 mat_id;
} entity_t;

#endif //ENTITY_H
