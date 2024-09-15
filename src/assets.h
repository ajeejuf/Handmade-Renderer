
#ifndef ASSETS_H
#define ASSETS_H

typedef enum asset_type_t {
    ASSET_SHADER,
} asset_type_t; 

typedef enum shader_type_t {
    VERTEX_SHADER,
    FRAGMENT_SHADER,
    COMPUTE_SHADER,
} shader_type_t;

typedef struct shader_entry_t {
    char **fns;
    shader_type_t *types;
    u32 count;
} shader_entry_t;

typedef struct asset_entry_t {
    asset_type_t type;
    
    union {
        shader_entry_t shader;
    };
} asset_entry_t;

typedef struct asset_manager_t {
    STACK(asset_entry_t) *entries;
} asset_manager_t; 

#endif //ASSETS_H
